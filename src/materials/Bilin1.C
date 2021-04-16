//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Bilin1.h"

#include "Function.h"
#include "ElasticityTensorTools.h"

registerMooseObject("TensorMechanicsApp", Bilin1);

InputParameters
Bilin1::validParams()
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
  params.addParam<Real>("deterioration_constant", 0.0, "Deterioration slope");
  params.addParam<Real>("peak_strength", 100000000000000.0, "Value of peak strength of a material"
                                                          "after which deterioration starts");
  params.addCoupledVar("temperature", 0.0, "Coupled Temperature");
  params.addDeprecatedParam<std::string>(
      "plastic_prepend",
      "",
      "String that is prepended to the plastic_strain Material Property",
      "This has been replaced by the 'base_name' parameter");
  params.set<std::string>("effective_inelastic_strain_name") = "effective_plastic_strain";

  return params;
}

Bilin1::Bilin1(const InputParameters & parameters)
  : RadialReturnStressUpdate(parameters),
    _plastic_prepend(getParam<std::string>("plastic_prepend")),
    _yield_stress(getParam<Real>("yield_stress")),
    _hardening_constant(getParam<Real>("hardening_constant")),
    _det_constant(getParam<Real>("deterioration_constant")),
    _peak_strength(getParam<Real>("peak_strength")),
    _yield_condition(-1.0), // set to a non-physical value to catch uninitalized yield condition
    _hardening_slope(0.0),
    _yield(declareProperty<Real>("yield")),
    _yield_old(getMaterialPropertyOld<Real>("yield")),
    _plastic_strain(
        declareProperty<RankTwoTensor>(_base_name + _plastic_prepend + "plastic_strain")),
    _plastic_strain_old(
        getMaterialPropertyOld<RankTwoTensor>(_base_name + _plastic_prepend + "plastic_strain")),
    _total_strain(getMaterialProperty<RankTwoTensor>(_base_name + "total_strain")),
    _total_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "total_strain")),
    _back_stress(declareProperty<RankTwoTensor>("back_stress")),
    _back_stress_old(getMaterialPropertyOld<RankTwoTensor>("back_stress")),
    _back_test(declareProperty<RankTwoTensor>("back_test")),
    _back_test_old(getMaterialPropertyOld<RankTwoTensor>("back_test")),
    _det_back_stress(declareProperty<RankTwoTensor>("det_back_stress")),
    _det_back_stress_old(getMaterialPropertyOld<RankTwoTensor>("det_back_stress")),
    _effective_stress(declareProperty<RankTwoTensor>("effective_stress")),
    _effective_stress_old(getMaterialPropertyOld<RankTwoTensor>("effective_stress")),
    _effective_inelastic_strain_older(this->template getMaterialPropertyOlder<Real>(
      this->_base_name +
      this->template getParam<std::string>("effective_inelastic_strain_name"))),
    _damage(declareProperty<bool>("damage")),
    _damage_old(getMaterialPropertyOld<bool>("damage")),
    _damagepos(declareProperty<bool>("damagepos")),
    _damagepos_old(getMaterialPropertyOld<bool>("damagepos")),
    _damageneg(declareProperty<bool>("damageneg")),
    _damageneg_old(getMaterialPropertyOld<bool>("damageneg")),
    _maxpos(declareProperty<Real>("maxpos")),
    _maxpos_old(getMaterialPropertyOld<Real>("maxpos")),
    _maxneg(declareProperty<Real>("maxneg")),
    _maxneg_old(getMaterialPropertyOld<Real>("maxneg")),
    _strain_max(declareProperty<Real>("strain_max")),
    _strain_max_old(getMaterialPropertyOld<Real>("strain_max")),
    _strain_min(declareProperty<Real>("strain_min")),
    _strain_min_old(getMaterialPropertyOld<Real>("strain_min")),
    _th_pos(declareProperty<Real>("th_pos")),
    _th_pos_old(getMaterialPropertyOld<Real>("th_pos")),
    _th_neg(declareProperty<Real>("th_neg")),
    _th_neg_old(getMaterialPropertyOld<Real>("th_neg")),
    _istestpos(declareProperty<bool>("istestpos")),
    _istestpos_old(getMaterialPropertyOld<bool>("istestpos")),
    _istestneg(declareProperty<bool>("istestneg")),
    _istestneg_old(getMaterialPropertyOld<bool>("istestneg"))

{
  if (parameters.isParamSetByUser("yield_stress") && _yield_stress <= 0.0)
    mooseError("Yield stress must be greater than zero");

  if (!parameters.isParamSetByUser("hardening_constant") && !isParamValid("hardening_function"))
    mooseError("Either hardening_constant or hardening_function must be defined");

  if (parameters.isParamSetByUser("hardening_constant") && isParamValid("hardening_function"))
    mooseError(
        "Only the hardening_constant or only the hardening_function can be defined but not both");
}

void
Bilin1::initQpStatefulProperties()
{
  _back_stress[_qp].zero();
  _plastic_strain[_qp].zero();
  _det_back_stress[_qp].zero();
  input_dt = 0.0;
  _damage[_qp] = false;
  _back_test[_qp].zero();
  _damagepos[_qp] = false;
  _damageneg[_qp] = false;
  _maxpos[_qp] = _peak_strength;
  _maxneg[_qp] = _peak_strength;
  _strain_max[_qp] = 0.0;
  _strain_min[_qp] = 0.0;
  _th_pos[_qp] = 0.0;
  _th_neg[_qp] = 0.0;
  _hardening_slope = _hardening_constant;
  _yield[_qp] = _yield_stress;
  _istestpos[_qp] = false;
  _istestneg[_qp] = false;

}

void
Bilin1::propagateQpStatefulProperties()
{
  _back_stress[_qp] = _back_stress_old[_qp];
  _plastic_strain[_qp] = _plastic_strain_old[_qp];


  propagateQpStatefulPropertiesRadialReturn();
}

void
Bilin1::updateState(RankTwoTensor & strain_increment,
                                      RankTwoTensor & inelastic_strain_increment,
                                      const RankTwoTensor & rotation_increment,
                                      RankTwoTensor & stress_new,
                                      const RankTwoTensor & stress_old,
                                      const RankFourTensor & elasticity_tensor,
                                      const RankTwoTensor & elastic_strain_old,
                                      bool compute_full_tangent_operator,
                                      RankFourTensor & tangent_operator)
{
   // std::cout << "\n\n\n***************************\n";
   // std::cout << "*** updateState() ***";
   // std::cout << "\n***************************\n";
   //
   // std::cout << "For QP = ******************"<<_qp<<" **********************\n"; /* << (_q_point[_qp])(0) << ", ";
   // std::cout << (_q_point[_qp])(1) << ", " << (_q_point[_qp])(2) << "):\n\n";*/
   //


    _damage[_qp] = _damage_old[_qp];
    _back_stress[_qp] = _back_stress_old[_qp];
    _det_back_stress[_qp] = _det_back_stress_old[_qp];
    _damagepos[_qp] = _damagepos_old[_qp];
    _damageneg[_qp] = _damageneg_old[_qp];
    _maxpos[_qp] = _maxpos_old[_qp];
    _maxneg[_qp] = _maxneg_old[_qp];
    _strain_max[_qp] = _strain_max_old[_qp];
    _strain_min[_qp] = _strain_min_old[_qp];
    _th_pos[_qp] = _th_pos_old[_qp];
    _th_neg[_qp] = _th_neg_old[_qp];
    _yield[_qp] = _yield_old[_qp];
    _istestpos[_qp] = _istestpos_old[_qp];
    _istestneg[_qp] = _istestneg_old[_qp];
    _back_test[_qp] = _back_test_old[_qp];



    RankTwoTensor backstress;
    RankTwoTensor zero_tensor;
    zero_tensor.zero();
    old = computeEffectiveStress(stress_old);
    s_new = computeEffectiveStress(stress_new);
    effective_strain = computeEffectiveStrain(_total_strain[_qp]);
    effective_strain_old = computeEffectiveStrain(_total_strain_old[_qp]);

    direction = MathUtils::sign(stress_old.thirdInvariant());
    strain_dir = MathUtils::sign(_total_strain[_qp].thirdInvariant());
    strain_dir_old = MathUtils::sign(_total_strain_old[_qp].thirdInvariant());


    // check if the deterioration starts
    if(direction == 1 && MooseUtils::absoluteFuzzyGreaterEqual(old, _maxpos[_qp],1e-8) || direction == -1 && MooseUtils::absoluteFuzzyGreaterEqual(old, _maxneg[_qp],1e-8))
    {
      // std::cout<<"************ if 1 called *********************\n";
      _damage[_qp] = true;
    }

    // if(_damage[_qp] == true && (strain_dir == 1 && direction == -1 || strain_dir == -1 && direction == 1))
    // {
    //   mooseError("zero residual stress reached in Bilinear Plasticity");
    // }

    // if(_damage[_qp] == true && MooseUtils::absoluteFuzzyGreaterThan((old - computeEffectiveStress(_back_stress[_qp])), _yield_stress))
    // {
    //   mooseError("strength limit reached in Bilinear Plasticity");
    // }

    // check for start of unloading and set damage status to false
    if(_damage[_qp] == true && MooseUtils::absoluteFuzzyLessThan(s_new, old,1e-8))
    {
      // std::cout<<"************ if 2 called *********************\n";
      _damage[_qp] = false;

      if(direction == 1)
      {
        // std::cout<<"************ if 21 called *********************\n";
        if(old < _maxpos[_qp])
        {
          // std::cout<<"************ if 211 called *********************\n";
          _maxpos[_qp] = old;
        }
        _th_pos[_qp] = effective_strain_old;
        _damagepos[_qp] = true;
        if(_istestpos[_qp] == true)
        {
          // std::cout<<"************** if 213 ***********\n";
          _back_stress[_qp] = _back_test[_qp];
          _back_test[_qp].zero();
          _istestpos[_qp] = false;
        }
      }
      if(direction == -1)
      {
        // std::cout<<"************ if 22 called *********************\n";
        if(old < _maxneg[_qp])
        {
          // std::cout<<"************ if 221 called *********************\n";
          _maxneg[_qp] = old;
        }
        _th_neg[_qp] = effective_strain_old;
        _damageneg[_qp] = true;
        if(_istestneg[_qp] == true)
        {
          // std::cout<<"************** if 223 ***********\n";
          _back_stress[_qp] = _back_test[_qp];
          _back_test[_qp].zero();
          _istestneg[_qp] = false;
        }
      }
    }

    if(MooseUtils::absoluteFuzzyLessThan(s_new,old))
    {
      if(direction == 1)
      {
        if(effective_strain_old > _strain_max[_qp])
        {
          // std::cout<<"************** if 212 ***********\n";
          _strain_max[_qp] = effective_strain_old;
        }
      }
      if(direction == -1)
      {
        if(effective_strain_old > _strain_min[_qp])
        {
          // std::cout<<"************** if 222 called ***********\n";
          _strain_min[_qp] = effective_strain_old;
        }
      }
    }


   RankTwoTensor deviatoric_trial_stress = stress_new.deviatoric();
   backstress = _det_back_stress_old[_qp];

   if(backstress == zero_tensor && _damage[_qp] == false || _damage_old[_qp] == false && _damage[_qp] ==true || MooseUtils::absoluteFuzzyLessThan(s_new,old))
   {
     // std::cout<<"************ if 3 called *********************\n";
     backstress.zero();
     _effective_stress[_qp] = deviatoric_trial_stress - _back_stress[_qp];
   }
   else
   {
     // std::cout<<"************ else 3 called *********************\n";
     _effective_stress[_qp] = deviatoric_trial_stress - backstress;
   }

   if(MathUtils::sign(_effective_stress[_qp].thirdInvariant()) == -1 && MathUtils::sign(_effective_stress_old[_qp].thirdInvariant()) == 1 && direction == 1)
   {
     // std::cout<<"************ if 4 called *********************\n";
     if(_damageneg[_qp] == false)
     {
       // std::cout<<"************ if 41 called *********************\n";
       _yield[_qp] = _yield_stress;
     }
     if(_yield[_qp] != _yield_stress)
     {
       _yield[_qp] = _yield_stress;
     }
     if(_damageneg[_qp] == true && (_maxneg[_qp] + computeEffectiveStress(_back_stress[_qp])) < _yield_stress)
     {
       // std::cout<<"************ if 42 called *********************\n";
       _istestneg[_qp] = true;
       if(!(_back_stress[_qp] == zero_tensor))
       {
         // std::cout<<"************ if 5****** called *********************\n";
         _back_test[_qp] = _back_stress[_qp];
       }
       _yield[_qp] = _maxneg[_qp];
       _back_stress[_qp].zero();
     }
   }

   if(MathUtils::sign(_effective_stress[_qp].thirdInvariant()) == 1 && MathUtils::sign(_effective_stress_old[_qp].thirdInvariant()) == -1 && direction == -1)
   {
     // std::cout<<"************ if 5 called *********************\n";
     if(_damagepos[_qp] == false)
     {
       // std::cout<<"************ if 51 called *********************\n";
       _yield[_qp] = _yield_stress;
     }
     if(_yield[_qp] != _yield_stress)
     {
       _yield[_qp] = _yield_stress;
     }
     if(_damagepos[_qp] == true && (_maxpos[_qp] + computeEffectiveStress(_back_stress[_qp])) < _yield_stress)
     {
       // std::cout<<"************ if 52 called *********************\n";
       _istestpos[_qp] = true;
       if(!(_back_stress[_qp] == zero_tensor))
       {
         // std::cout<<"************ if 5****** called *********************\n";
         _back_test[_qp] = _back_stress[_qp];
       }
       _yield[_qp] = _maxpos[_qp];
       _back_stress[_qp].zero();
     }
   }

      // Set the value of 3 * shear modulus for use as a reference residual value
   _three_shear_modulus = 3.0 * ElasticityTensorTools::getIsotropicShearModulus(elasticity_tensor);
   Real dev_trial_stress_squared = _effective_stress[_qp].doubleContraction(_effective_stress[_qp]);
   Real effective_trial_stress = std::sqrt(3.0 / 2.0 * dev_trial_stress_squared);

   computeStressInitialize(effective_trial_stress, elasticity_tensor);

   if(_yield_condition > 0)
   {
     // std::cout<<"************ if 8 called *********************\n";
     if(_damage[_qp] == false)
     {
       // std::cout<<"************ if 81 called *********************\n";
       _hardening_slope = _hardening_constant;
       _scalar_effective_inelastic_strain = 0.0;
       if (!MooseUtils::absoluteFuzzyEqual(effective_trial_stress, 0.0))
       {
         returnMappingSolve(effective_trial_stress, _scalar_effective_inelastic_strain, _console);
         if (_scalar_effective_inelastic_strain != 0.0)
         {
           inelastic_strain_increment =
               (deviatoric_trial_stress - _back_stress[_qp]) *
               (1.5 * _scalar_effective_inelastic_strain / effective_trial_stress);

           _back_stress[_qp] = _back_stress_old[_qp] + (2.0/3.0 * _hardening_slope * inelastic_strain_increment);
         }
         else
         {
           inelastic_strain_increment.zero();
         }
      }
      // might be unnecessary
      _det_back_stress[_qp].zero();
     }
     if(_damage[_qp] == true)
     {
       // std::cout<<"************ if 82 called *********************\n";
       if(strain_dir == 1 && effective_strain >= _strain_max[_qp] || strain_dir == -1 && effective_strain >= _strain_min[_qp])
       {
         // std::cout<<"************ if 82a called *********************\n";
         _hardening_slope = _det_constant;
       }
       else
       {
         // std::cout<<"************ else 82a called *********************\n";
         _hardening_slope = 0;
       }
       //set backstress for the first time step after deterioration
       if(_damage_old[_qp] == false && _damage[_qp] == true)
       {
         // std::cout<<"************** if 821 called ***********\n";
         backstress = _back_stress[_qp];
       }
       else
       {
         // std::cout<<"************** else 821 called ***********\n";
         backstress = backstress;
       }
       _scalar_effective_inelastic_strain = 0.0;
       if (!MooseUtils::absoluteFuzzyEqual(effective_trial_stress, 0.0))
       {
         returnMappingSolve(effective_trial_stress, _scalar_effective_inelastic_strain, _console);
         if (_scalar_effective_inelastic_strain != 0.0)
         {
           inelastic_strain_increment =
           (deviatoric_trial_stress - backstress) *
           (1.5 * _scalar_effective_inelastic_strain / effective_trial_stress);

           _det_back_stress[_qp] = backstress + (2.0/3.0) * _hardening_slope * inelastic_strain_increment;
           _back_stress[_qp] = _back_stress_old[_qp] + (2.0/3.0 * _hardening_constant * inelastic_strain_increment);
           if(_istestneg[_qp] == true || _istestpos[_qp] == true)
           {
             // std::cout<<"************** if 8211 ***********\n";
             _back_test[_qp] = _back_test[_qp] + (2.0/3.0 * _hardening_constant * inelastic_strain_increment);
           }
         }
         else
         {
           inelastic_strain_increment.zero();
         }
       }
     }
   }
   else
   {
     // std::cout<<"************ else 8 called ***************\n";
     inelastic_strain_increment.zero();
     _det_back_stress[_qp].zero();
   }

   strain_increment -= inelastic_strain_increment;

   // std::cout<<"elastic strain inc = "<<strain_increment<<"\n\n";

  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + _scalar_effective_inelastic_strain;

  // Use the old elastic strain here because we require tensors used by this class
  // to be isotropic and this method natively allows for changing in time
  // elasticity tensors

  stress_new = elasticity_tensor * (strain_increment + elastic_strain_old);

   // std::cout<<"bs = "<<_back_stress[_qp]<<"\n\n";
   // std::cout<<"d_bs = "<<_det_back_stress[_qp]<<"\n\n";
   // std::cout<<"backstress = "<<backstress<<"\n\n";
   // std::cout<<"back test = "<<_back_test[_qp]<<"\n\n";
   //
   // std::cout<<"damage? = "<<_damage[_qp]<<"\n";
   // std::cout<<"pos damage? = "<<_damagepos[_qp]<<"\n";
   // std::cout<<"neg damage? = "<<_damageneg[_qp]<<"\n";
   // std::cout<<"test pos? = "<<_istestpos[_qp]<<"\n";
   // std::cout<<"test neg? = "<<_istestneg[_qp]<<"\n\n";
   //
   // std::cout<<"old = "<<old<<"\n";
   // std::cout<<"new = "<<s_new<<"\n";
   // std::cout<<"pos limit = "<<_maxpos[_qp]<<"\n";
   // std::cout<<"neg limit = "<<_maxneg[_qp]<<"\n\n";
   //
   // std::cout<<"strain old = "<<effective_strain<<"\n";
   // std::cout<<"strain = "<<effective_strain_old<<"\n";
   // std::cout<<"max strain = "<<_strain_max[_qp]<<"\n";
   // std::cout<<"min strain = "<<_strain_min[_qp]<<"\n";
   // std::cout<<"th pos = "<<_th_pos[_qp]<<"\n";
   // std::cout<<"th neg = "<<_th_neg[_qp]<<"\n\n";
   //
   // std::cout<<"hard = "<<_hardening_slope<<"\n\n";
   //
   // std::cout<<"yield stress = "<<_yield_stress<<"\n";
   // std::cout<<"yield = "<<_yield[_qp]<<"\n";
   // std::cout<<"yield condition = "<<_yield_condition<<"\n\n";
   //
   // std::cout<<"dirn = "<<MathUtils::sign(_effective_stress[_qp].thirdInvariant())<<"\n";
   // std::cout<<"dirn old = "<<MathUtils::sign(_effective_stress_old[_qp].thirdInvariant())<<"\n\n";


 }


 void
 Bilin1::computeStressInitialize(const Real & effective_trial_stress,
                                                          const RankFourTensor & elasticity_tensor)
 {
   _yield_condition = effective_trial_stress - _yield[_qp];

   _plastic_strain[_qp] = _plastic_strain_old[_qp];

   // std::cout<<"plastic strain = " << _plastic_strain[_qp] <<"\n";

 }

 Real
 Bilin1::computeResidual(const Real effective_trial_stress,
                                                  const Real scalar)
 {
   mooseAssert(_yield_condition != -1.0,
               "the yield stress was not updated by computeStressInitialize");

   if (_yield_condition > 0.0)
   {
     return (effective_trial_stress - scalar * _hardening_slope - _yield[_qp]) /
                _three_shear_modulus - scalar;
   }

   return 0.0;
 }

 Real
 Bilin1::computeReferenceResidual(const Real  & effective_trial_stress,
                                                    const Real & scalar_effective_inelastic_strain)
 {
   return (effective_trial_stress - _hardening_slope * scalar_effective_inelastic_strain) / _three_shear_modulus -
               scalar_effective_inelastic_strain;
 }

 Real
 Bilin1::computeDerivative(const Real /*effective_trial_stress*/,
                                                    const Real /*scalar*/)
 {
   if (_yield_condition > 0.0)
     return -1.0 - _hardening_slope / _three_shear_modulus;

   return 1.0;
 }

 void
 Bilin1::iterationFinalize(Real scalar)
 {
   if (_yield_condition > 0.0)
   {
     /* might have to update backstress at some point though let us see for now */
   }
 }

 void
 Bilin1::computeStressFinalize(
     const RankTwoTensor & plastic_strain_increment)
 {
   _plastic_strain[_qp] += plastic_strain_increment;
 }


Real
Bilin1::computeTimeStepLimit()
{
  if(_dt < _dt_old)
    if(_dt_old > input_dt)
      input_dt = _dt_old;

  Real incr = _effective_inelastic_strain_old[_qp] - _effective_inelastic_strain_older[_qp];
  Real test = old + 2 * (_dt/_dt_old) * incr * _hardening_slope;
  Real current = (_dt/_dt_old) * _hardening_slope* incr + old;

  // std::cout<<"old = "<<old<<"\n";
  // // std::cout<<"older = "<<older<<"\n";
  // std::cout<<"incr = "<<incr<<"\n";
  // std::cout<<"current = "<<current<<"\n";
  // std::cout<<"test = "<<test<<"\n\n";
  // std::cout<<"dt = "<<_dt * (264.0 - current)/(test - current)<<"\n\n";

  if (test > _peak_strength && MooseUtils::absoluteFuzzyLessThan(current, _peak_strength,1e-8))
  {
    return _dt * (_peak_strength - current)/(test - current);
  }
  else if (_dt < _dt_old && _dt_old == input_dt)
  {
    return (_dt_old - _dt);
  }
  else if (_dt < _dt_old || _dt == _dt_old || _dt > _dt_old)
  {
    // if(2 * _dt < input_dt)
    //   return 2 * _dt;
    // else
      return input_dt;
  }
  else
    return _dt;
}

Real
Bilin1::computeEffectiveStress(const RankTwoTensor stress)
{
  RankTwoTensor deviatoric_stress = stress.deviatoric();
  Real dev_stress_squared =
      deviatoric_stress.doubleContraction(deviatoric_stress);

  return std::sqrt(3.0 / 2.0 * dev_stress_squared);
}

Real
Bilin1::computeEffectiveStrain(const RankTwoTensor strain)
{
  RankTwoTensor deviatoric_strain = strain.deviatoric();
  Real dev_strain_squared =
      deviatoric_strain.doubleContraction(deviatoric_strain);

  return std::sqrt(2.0 / 3.0 * dev_strain_squared);
}
