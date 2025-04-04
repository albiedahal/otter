//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RadialReturnStressUpdate.h"

/**
 * This class uses the Discrete material in a radial return Kinematic plasticity
 * model.  This class is one of the basic radial return constitutive models;
 * more complex constitutive models combine creep and plasticity.
 *
 * This class inherits from RadialReturnStressUpdate and must be used
 * in conjunction with ComputeReturnMappingStress.  This class calculates
 * an effective trial stress, an effective scalar plastic strain
 * increment, and the derivative of the scalar effective plastic strain increment;
 * these values are passed to the RadialReturnStressUpdate to compute
 * the radial return stress increment.  This Kinematic plasticity class also
 * computes the plastic strain as a stateful material property.
 *
 * This class is based on the implicit integration algorithm in F. Dunne and N.
 * Petrinic's Introduction to Computational Plasticity (2004) Oxford University
 * Press, pg. 146 - 149.
 */

class KinematicPlasticityStressUpdate : public RadialReturnStressUpdate
{
public:
  static InputParameters validParams();

  KinematicPlasticityStressUpdate(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void propagateQpStatefulProperties() override;

  virtual void updateState(RankTwoTensor & strain_increment,
                         RankTwoTensor & inelastic_strain_increment,
                         const RankTwoTensor & rotation_increment,
                         RankTwoTensor & stress_new,
                         const RankTwoTensor & stress_old,
                         const RankFourTensor & elasticity_tensor,
                         const RankTwoTensor & elastic_strain_old,
                         bool compute_full_tangent_operator,
                         RankFourTensor & tangent_operator) override;

  virtual void computeStressInitialize(const Real & effective_trial_stress,
                                       const RankFourTensor & elasticity_tensor) override;
  virtual Real computeResidual(const Real effective_trial_stress, const Real scalar) override;
  virtual Real computeReferenceResidual(const Real & effective_trial_stress, const Real & scalar_effective_inelastic_strain) override;
  virtual Real computeDerivative(const Real effective_trial_stress, const Real scalar) override;
  virtual void iterationFinalize(Real scalar) override;
  virtual void computeStressFinalize(const RankTwoTensor & plastic_strain_increment) override;

  virtual void computeYieldStress(const RankFourTensor & elasticity_tensor);
  virtual Real computeHardeningDerivative(Real scalar);

  /// a string to prepend to the plastic strain Material Property name
  const std::string _plastic_prepend;

  const Function * _yield_stress_function;
  Real _yield_stress;
  const Real _hardening_constant;
  const Function * const _hardening_function;

  Real _yield_condition;
  Real _hardening_slope;

  /// plastic strain in this model
  MaterialProperty<RankTwoTensor> & _plastic_strain;

  /// old value of plastic strain
  const MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  Real _youngs_modulus;

  MaterialProperty<RankTwoTensor> & _back_stress;
  const MaterialProperty<RankTwoTensor> & _back_stress_old;
  const VariableValue & _temperature;
};
