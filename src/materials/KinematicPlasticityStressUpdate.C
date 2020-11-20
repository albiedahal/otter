//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KinematicPlasticityStressUpdate.h"

#include "Function.h"
#include "ElasticityTensorTools.h"

registerMooseObject("TensorMechanicsApp", KinematicPlasticityStressUpdate);

InputParameters
KinematicPlasticityStressUpdate::validParams()
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
  params.addCoupledVar("temperature", 0.0, "Coupled Temperature");
  params.addDeprecatedParam<std::string>(
      "plastic_prepend",
      "",
      "String that is prepended to the plastic_strain Material Property",
      "This has been replaced by the 'base_name' parameter");
  params.set<std::string>("effective_inelastic_strain_name") = "effective_plastic_strain";

  return params;
}

KinematicPlasticityStressUpdate::KinematicPlasticityStressUpdate(const InputParameters & parameters)
  : RadialReturnStressUpdate(parameters),
    _plastic_prepend(getParam<std::string>("plastic_prepend")),
    _yield_stress_function(
        isParamValid("yield_stress_function") ? &getFunction("yield_stress_function") : NULL),
    _yield_stress(getParam<Real>("yield_stress")),
    _hardening_constant(getParam<Real>("hardening_constant")),
    _hardening_function(isParamValid("hardening_function") ? &getFunction("hardening_function")
                                                           : NULL),
    _yield_condition(-1.0), // set to a non-physical value to catch uninitalized yield condition
    _hardening_slope(0.0),
    _plastic_strain(
        declareProperty<RankTwoTensor>(_base_name + _plastic_prepend + "plastic_strain")),
    _plastic_strain_old(
        getMaterialPropertyOld<RankTwoTensor>(_base_name + _plastic_prepend + "plastic_strain")),
    _back_stress(declareProperty<RankTwoTensor>("back_stress")),
    _back_stress_old(getMaterialPropertyOld<RankTwoTensor>("back_stress")),
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
KinematicPlasticityStressUpdate::initQpStatefulProperties()
{
  _back_stress[_qp].zero();
  _plastic_strain[_qp].zero();
}

void
KinematicPlasticityStressUpdate::propagateQpStatefulProperties()
{
  _back_stress[_qp] = _back_stress_old[_qp];
  _plastic_strain[_qp] = _plastic_strain_old[_qp];

  propagateQpStatefulPropertiesRadialReturn();
}

void
KinematicPlasticityStressUpdate::updateState(RankTwoTensor & strain_increment,
                                      RankTwoTensor & inelastic_strain_increment,
                                      const RankTwoTensor & /*rotation_increment*/,
                                      RankTwoTensor & stress_new,
                                      const RankTwoTensor & stress_old,
                                      const RankFourTensor & elasticity_tensor,
                                      const RankTwoTensor & elastic_strain_old,
                                      bool compute_full_tangent_operator,
                                      RankFourTensor & tangent_operator)
{
  // compute the deviatoric trial stress and trial strain from the current intermediate
  // configuration

  RankTwoTensor deviatoric_trial_stress = stress_new.deviatoric();

  _back_stress[_qp] = _back_stress_old[_qp];

  std::cout<<"_back_stress = " << _back_stress[_qp] <<"\n";
  std::cout<<"sig_tr_' - x_t = " << deviatoric_trial_stress - _back_stress_old[_qp] <<"\n\n";

  // compute the effective trial stress
  Real dev_trial_stress_squared =
      deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress - _back_stress[_qp]);
  Real effective_trial_stress = std::sqrt(3.0 / 2.0 * dev_trial_stress_squared);

  std::cout<<"scalar eff trial stress = " << effective_trial_stress <<"\n";


  // Set the value of 3 * shear modulus for use as a reference residual value
  _three_shear_modulus = 3.0 * ElasticityTensorTools::getIsotropicShearModulus(elasticity_tensor);
  _youngs_modulus = ElasticityTensorTools::getIsotropicYoungsModulus(elasticity_tensor);

  computeStressInitialize(effective_trial_stress, elasticity_tensor);

  // Use Newton iteration to determine the scalar effective inelastic strain increment
  _scalar_effective_inelastic_strain = 0.0;
  if (!MooseUtils::absoluteFuzzyEqual(effective_trial_stress, 0.0))
  {
    Real test_strain = (effective_trial_stress - _yield_stress)/(_three_shear_modulus + _hardening_slope);
    std::cout<<"test call befor return solve = "<<test_strain<<"\n";

    returnMappingSolve(effective_trial_stress, _scalar_effective_inelastic_strain, _console);
    if (_scalar_effective_inelastic_strain != 0.0)
    {
      inelastic_strain_increment =
          (deviatoric_trial_stress - _back_stress[_qp]) *
          (1.5 * _scalar_effective_inelastic_strain / effective_trial_stress);

      // _back_stress[_qp] = _back_stress_old[_qp] + 2.0 * inelastic_strain_increment * _hardening_slope / 3.0;
      // _back_stress[_qp] = _back_stress_old[_qp] + std::sqrt(2.0/3.0) * _scalar_effective_inelastic_strain * _hardening_slope * (deviatoric_trial_stress - _back_stress[_qp])/std::sqrt(dev_trial_stress_squared);


      std::cout<<"del_p = " << _scalar_effective_inelastic_strain <<"\n\n";
      std::cout<<"inelastic strain inc = " << inelastic_strain_increment <<"\n\n";
    }


    else
      inelastic_strain_increment.zero();
  }
  else
    inelastic_strain_increment.zero();

  std::cout<<"strain inc = " << strain_increment <<"\n\n";


  strain_increment -= inelastic_strain_increment;

  std::cout<<"elastic strain inc = " << strain_increment <<"\n\n";


  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + _scalar_effective_inelastic_strain;

  // Use the old elastic strain here because we require tensors used by this class
  // to be isotropic and this method natively allows for changing in time
  // elasticity tensors

  std::cout<<"stress old = " << stress_old <<"\n\n";
  std::cout<<"elastic strain old = " << elastic_strain_old <<"\n\n";


  stress_new = elasticity_tensor * (strain_increment + elastic_strain_old);

  std::cout<<"new stress = " << stress_new <<"\n\n";


  computeStressFinalize(inelastic_strain_increment);

  _back_stress[_qp] = (2.0/3.0) * _plastic_strain[_qp] * _hardening_slope;


  computeTangentOperator(
      effective_trial_stress, stress_new, compute_full_tangent_operator, tangent_operator);
}

void
KinematicPlasticityStressUpdate::computeStressInitialize(const Real effective_trial_stress,
                                                         const RankFourTensor & elasticity_tensor)
{
  computeYieldStress(elasticity_tensor);

  _yield_condition = effective_trial_stress - _yield_stress;

  std::cout<<"yield condition = " << _yield_condition <<"\n";

  _plastic_strain[_qp] = _plastic_strain_old[_qp];

  std::cout<<"plastic strain = " << _plastic_strain[_qp] <<"\n";

}

Real
KinematicPlasticityStressUpdate::computeResidual(const Real effective_trial_stress,
                                                 const Real scalar)
{
  mooseAssert(_yield_condition != -1.0,
              "the yield stress was not updated by computeStressInitialize");

  if (_yield_condition > 0.0)
  {
    _hardening_slope = computeHardeningDerivative(scalar);

    return (effective_trial_stress - scalar * _hardening_slope - _yield_stress) /
               _three_shear_modulus - scalar;
  }

  return 0.0;
}

Real
KinematicPlasticityStressUpdate::computeReferenceResidual(const Real effective_trial_stress,
                                                   const Real scalar_effective_inelastic_strain)
{
  return (effective_trial_stress - _hardening_slope * scalar_effective_inelastic_strain) / _three_shear_modulus -
              scalar_effective_inelastic_strain;
}

Real
KinematicPlasticityStressUpdate::computeDerivative(const Real /*effective_trial_stress*/,
                                                   const Real /*scalar*/)
{
  if (_yield_condition > 0.0)
    return -1.0 - _hardening_slope / _three_shear_modulus;

  return 1.0;
}

void
KinematicPlasticityStressUpdate::iterationFinalize(Real scalar)
{
  if (_yield_condition > 0.0)
  {
    /* might have to update backstress at some point though let us see for now */
  }
}

void
KinematicPlasticityStressUpdate::computeStressFinalize(
    const RankTwoTensor & plastic_strain_increment)
{
  _plastic_strain[_qp] += plastic_strain_increment;
}

Real KinematicPlasticityStressUpdate::computeHardeningDerivative(Real /*scalar*/)
{
  if (_hardening_function)
  {
    const Real strain_old = _effective_inelastic_strain_old[_qp];
    const Point p; // Always (0,0,0)

    return _hardening_function->timeDerivative(strain_old, p);
  }

  return _hardening_constant;
  // return ((_hardening_constant * _youngs_modulus)/(-_hardening_constant + _youngs_modulus));
}

void
KinematicPlasticityStressUpdate::computeYieldStress(const RankFourTensor & /*elasticity_tensor*/)
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
