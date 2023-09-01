//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CombinedHardeningStressUpdatel.h"

#include "Function.h"
#include "ElasticityTensorTools.h"

registerMooseObject("TensorMechanicsApp", CombinedHardeningStressUpdatel);

InputParameters
CombinedHardeningStressUpdatel::validParams()
{
  InputParameters params = RadialReturnStressUpdate::validParams();
  params.addClassDescription("This class uses the discrete material in a radial return Kinematic "
                             "plasticity model.  This class is one of the basic radial return "
                             "constitutive models, yet it can be used in conjunction with other "
                             "creep and plasticity materials for more complex simulations.");
  // Linear strain hardening parameters
  params.addParam<FunctionName>("yield_stress_function",
                                "Yield stress as a function of temperature");
  params.addParam<Real>(
      "yield_stress", 0.0, "The point at which plastic strain begins accumulating");
  params.addParam<FunctionName>("hardening_function",
                                "True stress as a function of plastic strain");
  params.addParam<Real>("hardening_constant", 0.0, "Hardening slope");
  params.addParam<Real>("deterioration_constant", 0.0, "Softening slope");
  params.addParam<Real>("peak_strength",1000000000000,"Peak strength");
  params.addCoupledVar("temperature", 0.0, "Coupled Temperature");
  params.addDeprecatedParam<std::string>(
      "plastic_prepend",
      "",
      "String that is prepended to the plastic_strain Material Property",
      "This has been replaced by the 'base_name' parameter");
  params.set<std::string>("effective_inelastic_strain_name") = "effective_plastic_strain";

  return params;
}

CombinedHardeningStressUpdatel::CombinedHardeningStressUpdatel(const InputParameters & parameters)
  : RadialReturnStressUpdate(parameters),
    _plastic_prepend(getParam<std::string>("plastic_prepend")),
    _yield_stress_function(
        isParamValid("yield_stress_function") ? &getFunction("yield_stress_function") : NULL),
    _yield_stress(getParam<Real>("yield_stress")),
    _hardening_constant(getParam<Real>("hardening_constant")),
    _det_constant(getParam<Real>("deterioration_constant")),
    _peak_strength(getParam<Real>("peak_strength")),
    _yield_condition(-1.0), // set to a non-physical value to catch uninitalized yield condition
    _hardening_slope(0.0),
    _plastic_strain(
        declareProperty<RankTwoTensor>(_base_name + _plastic_prepend + "plastic_strain")),
    _plastic_strain_old(
        getMaterialPropertyOld<RankTwoTensor>(_base_name + _plastic_prepend + "plastic_strain")),
    _back_stress(declareProperty<RankTwoTensor>("back_stress")),
    _back_stress_old(getMaterialPropertyOld<RankTwoTensor>("back_stress")),
    _hardening_variable(declareProperty<Real>("hardening_variable")),
    _hardening_variable_old(getMaterialPropertyOld<Real>("hardening_variable")),
    _damage(declareProperty<bool>("damage")),
    _damage_old(getMaterialPropertyOld<bool>("damage")),
    _maxpos(declareProperty<Real>("maxpos")),
    _maxpos_old(getMaterialPropertyOld<Real>("maxpos")),
    _maxneg(declareProperty<Real>("maxneg")),
    _maxneg_old(getMaterialPropertyOld<Real>("maxneg")),
    _temperature(coupledValue("temperature"))
{
  if (parameters.isParamSetByUser("yield_stress") && _yield_stress <= 0.0)
    mooseError("Yield stress must be greater than zero");

  if (_yield_stress_function == NULL && !parameters.isParamSetByUser("yield_stress"))
    mooseError("Either yield_stress or yield_stress_function must be given");

  if (!parameters.isParamSetByUser("hardening_constant") && !isParamValid("hardening_function"))
    mooseError("Either hardening_constant or hardening_function must be defined");

  if (parameters.isParamSetByUser("hardening_constant") && isParamValid("hardening_function"))
    mooseError(
        "Only the hardening_constant or only the hardening_function can be defined but not both");
}

void
CombinedHardeningStressUpdatel::initQpStatefulProperties()
{
  _hardening_variable[_qp] = 0.0;
  _back_stress[_qp].zero();
  _plastic_strain[_qp].zero();
  _damage[_qp] = false;
  _maxpos[_qp] = _peak_strength;
  _maxneg[_qp] = _peak_strength;
}

void
CombinedHardeningStressUpdatel::propagateQpStatefulProperties()
{
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _back_stress[_qp] = _back_stress_old[_qp];
  _plastic_strain[_qp] = _plastic_strain_old[_qp];
  _damage[_qp] = _damage_old[_qp];
  _maxpos[_qp] = _maxpos_old[_qp];
  _maxneg[_qp] = _maxneg_old[_qp];

  propagateQpStatefulPropertiesRadialReturn();
}

void
CombinedHardeningStressUpdatel::updateState(RankTwoTensor & strain_increment,
                                      RankTwoTensor & inelastic_strain_increment,
                                      const RankTwoTensor & rotation_increment,
                                      RankTwoTensor & stress_new,
                                      const RankTwoTensor & stress_old,
                                      const RankFourTensor & elasticity_tensor,
                                      const RankTwoTensor & elastic_strain_old,
                                      bool compute_full_tangent_operator,
                                      RankFourTensor & tangent_operator)
{
  // compute the deviatoric trial stress and trial strain from the current intermediate
  // configuration
  // std::cout << "\n\n\n***************************\n";
  // std::cout << "*** updateState() ***";
  // std::cout << "\n***************************\n";
  //
  // std::cout << "For QP = "<<_qp<<" (" << (_q_point[_qp])(0) << ", ";
  // std::cout << (_q_point[_qp])(1) << ", " << (_q_point[_qp])(2) << "):\n\n";
  //
  // std::cout<<"trial stress = " << stress_new <<"\n";

  // std::cout<<"\n weight = "<<_JxW[_qp] * _coord[_qp]<<"\n";

  // std::cout<<"***************** start **************\n\n";

  old = MathUtils::round(computeEffectiveStress(stress_old) * 1e8)/1e8;
  s_new = MathUtils::round(computeEffectiveStress(stress_new) * 1e8)/1e8;
  direction = MathUtils::sign(stress_old.thirdInvariant());
  _damage[_qp] = _damage_old[_qp];
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _back_stress[_qp] = _back_stress_old[_qp];


  // check if the deterioration starts
  if(direction == 1 && MooseUtils::absoluteFuzzyGreaterEqual(old, _maxpos[_qp]) || direction == -1 && MooseUtils::absoluteFuzzyGreaterEqual(old, _maxneg[_qp]))
  {
    // std::cout<<"************ if 1 called *********************\n";
    _damage[_qp] = true;
  }

  // check for start of unloading and set damage status to false
  if(_damage[_qp] == true && MooseUtils::absoluteFuzzyLessThan(s_new, old))
  {
    // std::cout<<"************ if 2 called *********************\n";
    _damage[_qp] = false;

    // if(direction == 1)
    // {
      // std::cout<<"************ if 21 called *********************\n";
      if(old < _maxpos[_qp])
      {
        // std::cout<<"************ if 211 called *********************\n";
        _maxpos[_qp] = old;
      }
      //  if(effective_strain_old > _strain_max[_qp])
      // {
      //   // std::cout<<"************** if 212 ***********\n";
      //   _strain_max[_qp] = effective_strain_old;
      // }
    // }
    // if(direction == -1)
    // {
      // std::cout<<"************ if 22 called *********************\n";
      if(old < _maxneg[_qp])
      {
        // std::cout<<"************ if 221 called *********************\n";
        _maxneg[_qp] = old;
      }
      // if(effective_strain_old > _strain_min[_qp])
      // {
      //   // std::cout<<"************** if 222 called ***********\n";
      //   _strain_min[_qp] = effective_strain_old;
      // }
    // }
  }


  // std::cout<<" trial stress = "<<stress_new<<"\n\n";
  RankTwoTensor deviatoric_trial_stress = stress_new.deviatoric();

  _back_stress[_qp] = _back_stress_old[_qp];
  RankTwoTensor effective_stress = deviatoric_trial_stress - _back_stress[_qp];
  // std::cout<<"tensor s_tr' = "<<effective_stress<<"\n\n";

  // compute the effective trial stress
  Real dev_trial_stress_squared =
      effective_stress.doubleContraction(effective_stress);

  Real effective_trial_stress = std::sqrt(3.0 / 2.0 * dev_trial_stress_squared);
  // std::cout<<"s_e^tr = "<<effective_trial_stress<<"\n\n";

  // Set the value of 3 * shear modulus for use as a reference residual value
  _three_shear_modulus = 3.0 * ElasticityTensorTools::getIsotropicShearModulus(elasticity_tensor);

  computeStressInitialize(effective_trial_stress, elasticity_tensor);

  // Use Newton iteration to determine the scalar effective inelastic strain increment
  _scalar_effective_inelastic_strain = 0.0;
  if (!MooseUtils::absoluteFuzzyEqual(effective_trial_stress, 0.0))
  {
    returnMappingSolve(effective_trial_stress, _scalar_effective_inelastic_strain, _console);
    // std::cout<<"del_p = "<<_scalar_effective_inelastic_strain<<"\n\n";
    if (_scalar_effective_inelastic_strain != 0.0)
    {
      inelastic_strain_increment =
          (deviatoric_trial_stress - _back_stress[_qp]) *
          (1.5 * _scalar_effective_inelastic_strain / effective_trial_stress);

      // std::cout<< " del e^p = "<<inelastic_strain_increment<<"\n\n";

      _back_stress[_qp] = _back_stress_old[_qp] + (2.0/3.0 * _hardening_slope * inelastic_strain_increment);
      // std::cout<<"x = "<<_back_stress[_qp]<<"\n\n";
    }


    else
      inelastic_strain_increment.zero();
  }
  else
    inelastic_strain_increment.zero();

  strain_increment -= inelastic_strain_increment;

  // std::cout<<"del e^e = "<<strain_increment<<"\n\n";

  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + _scalar_effective_inelastic_strain;

  // Use the old elastic strain here because we require tensors used by this class
  // to be isotropic and this method natively allows for changing in time
  // elasticity tensors

  stress_new = elasticity_tensor * (strain_increment + elastic_strain_old);

  // std::cout<<"final stress = "<<stress_new<<"\n\n";

  computeStressFinalize(inelastic_strain_increment);
  computeTangentOperator(
      effective_trial_stress, stress_new, compute_full_tangent_operator, tangent_operator);
}


void
CombinedHardeningStressUpdatel::computeStressInitialize(const Real & effective_trial_stress,
                                                         const RankFourTensor & elasticity_tensor)
{
  computeYieldStress(elasticity_tensor);

  _yield_condition = effective_trial_stress - _hardening_variable_old[_qp] - _yield_stress;

  // std::cout<<"yield condition = " << _yield_condition <<"\n";
  // std::cout<<"s_e^tr while yield = "<<effective_trial_stress<<"\n\n";
  // std::cout<<"r while yield = "<<_hardening_variable_old[_qp]<<"\n\n";
  // std::cout<<"r new while yield = "<<_hardening_variable[_qp]<<"\n\n";


  _plastic_strain[_qp] = _plastic_strain_old[_qp];

  // std::cout<<"plastic strain = " << _plastic_strain[_qp] <<"\n";

}

Real
CombinedHardeningStressUpdatel::computeResidual(const Real effective_trial_stress,
                                                 const Real scalar)
{
  mooseAssert(_yield_condition != -1.0,
              "the yield stress was not updated by computeStressInitialize");

  if (_yield_condition > 0.0)
  {
    // std::cout<<"damage = "<<_damage[_qp]<<"\n\n";
    if(_damage[_qp] == true)
    {
      _hardening_slope = 0.0;
      _det_slope = _det_constant;
      if(std::abs(_hardening_variable_old[_qp])>0.95*_yield_stress && std::abs(_hardening_variable_old[_qp])>0)
        _det_slope = 0.0;
    }

    if(_damage[_qp] == false)
    {
      _hardening_slope = _hardening_constant;
      _det_slope = 0.0;
    }
    _hardening_variable[_qp] = _hardening_variable_old[_qp] + scalar * _det_slope;

    Real res = (effective_trial_stress - _hardening_variable[_qp] - _yield_stress) /
               (_three_shear_modulus + _hardening_slope) - scalar;
    // std::cout<<"res = "<<res<<"\n\n";

    return (effective_trial_stress - _hardening_variable[_qp] - _yield_stress) /
               (_three_shear_modulus + _hardening_slope) - scalar;
  }

  return 0.0;
}

Real
CombinedHardeningStressUpdatel::computeReferenceResidual(const Real  & effective_trial_stress,
                                                   const Real & scalar_effective_inelastic_strain)
{
  return (effective_trial_stress - _hardening_slope * scalar_effective_inelastic_strain) / _three_shear_modulus -
              scalar_effective_inelastic_strain;
}

Real
CombinedHardeningStressUpdatel::computeDerivative(const Real /*effective_trial_stress*/,
                                                   const Real /*scalar*/)
{
  if (_yield_condition > 0.0)
    return -1.0 - _det_slope / (_three_shear_modulus + _hardening_slope);

  return 1.0;
}

void
CombinedHardeningStressUpdatel::iterationFinalize(Real scalar)
{
  if (_yield_condition > 0.0)
  {
    _hardening_variable[_qp] = _hardening_variable_old[_qp] + scalar * _det_slope;
    // std::cout<<"r = "<<_hardening_variable[_qp]<<"\n\n";
  }
}

void
CombinedHardeningStressUpdatel::computeStressFinalize(
    const RankTwoTensor & plastic_strain_increment)
{
  _plastic_strain[_qp] += plastic_strain_increment;
}

void
CombinedHardeningStressUpdatel::computeYieldStress(const RankFourTensor & /*elasticity_tensor*/)
{
  if (_yield_stress_function)
  {
    const Point p;
    _yield_stress = _yield_stress_function->value(_temperature[_qp], p);

    if (_yield_stress <= 0.0)
      mooseError(
          "In ", _name, ": The calculated yield stress (", _yield_stress, ") is less than zero");
  }
}

Real
CombinedHardeningStressUpdatel::computeEffectiveStress(RankTwoTensor stress)
{
  RankTwoTensor deviatoric_stress = stress.deviatoric();
  Real dev_stress_squared =
      deviatoric_stress.doubleContraction(deviatoric_stress);

  return std::sqrt(3.0 / 2.0 * dev_stress_squared);
}
