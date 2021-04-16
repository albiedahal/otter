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

class Bilin1 : public RadialReturnStressUpdate
{
public:
  static InputParameters validParams();

  Bilin1(const InputParameters & parameters);

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

  virtual Real computeTimeStepLimit() override;
  virtual Real computeEffectiveStress(RankTwoTensor stress);
  virtual Real computeEffectiveStrain(RankTwoTensor strain);


  /// a string to prepend to the plastic strain Material Property name
  const std::string _plastic_prepend;

  Real _yield_stress;
  const Real _hardening_constant;
  const Real _det_constant;
  const Real _peak_strength;

  Real _yield_condition;
  Real _hardening_slope;

  /// yield stress used in each plstic iteration
  MaterialProperty<Real> & _yield;
  const MaterialProperty<Real> & _yield_old;

  /// plastic strain in this model
  MaterialProperty<RankTwoTensor> & _plastic_strain;

  /// old value of plastic strain
  const MaterialProperty<RankTwoTensor> & _plastic_strain_old;

  /// strain tensors
  const MaterialProperty<RankTwoTensor> & _total_strain;
  const MaterialProperty<RankTwoTensor> & _total_strain_old;


  Real _youngs_modulus;

  /// backstress tensor
  MaterialProperty<RankTwoTensor> & _back_stress;
  const MaterialProperty<RankTwoTensor> & _back_stress_old;

  /// backstress for when strength limit is reached
  MaterialProperty<RankTwoTensor> & _back_test;
  const MaterialProperty<RankTwoTensor> & _back_test_old;

  /// backstress for deteriorating part of response
  MaterialProperty<RankTwoTensor> & _det_back_stress;
  const MaterialProperty<RankTwoTensor> & _det_back_stress_old;


  MaterialProperty<RankTwoTensor> & _effective_stress;
  const MaterialProperty<RankTwoTensor> & _effective_stress_old;
  const MaterialProperty<Real> & _effective_inelastic_strain_older;

  Real input_dt;
  Real old;
  Real s_new;
  Real effective_strain;
  Real effective_strain_old;
  Real strain_dir;
  Real strain_dir_old;
  Real incremental_strain;

  MaterialProperty<bool> & _damage;
  const MaterialProperty<bool> & _damage_old;

  Real direction;

  /// flag to check if positive or negative side has deteriorated
  MaterialProperty<bool> & _damagepos;
  const MaterialProperty<bool> & _damagepos_old;
  MaterialProperty<bool> & _damageneg;
  const MaterialProperty<bool> & _damageneg_old;

  /// strength limits
  MaterialProperty<Real> & _maxpos;
  const MaterialProperty<Real> & _maxpos_old;
  MaterialProperty<Real> & _maxneg;
  const MaterialProperty<Real> & _maxneg_old;

  /// strain limits
  MaterialProperty<Real> & _strain_max;
  const MaterialProperty<Real> & _strain_max_old;
  MaterialProperty<Real> & _strain_min;
  const MaterialProperty<Real> & _strain_min_old;
  MaterialProperty<Real> & _th_pos;
  const MaterialProperty<Real> & _th_pos_old;
  MaterialProperty<Real> & _th_neg;
  const MaterialProperty<Real> & _th_neg_old;

  /// flag for if strength limit occurs before yield surface
  MaterialProperty<bool> & _istestpos;
  const MaterialProperty<bool> & _istestpos_old;
  MaterialProperty<bool> & _istestneg;
  const MaterialProperty<bool> & _istestneg_old;
};
