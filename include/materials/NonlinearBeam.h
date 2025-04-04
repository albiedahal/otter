//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankTwoTensor.h"

/**
 * NonlinearBeam computes forces and moments using elasticity
 */

class NonlinearBeam : public Material
{
public:
  static InputParameters validParams();

  NonlinearBeam(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;
  virtual void initQpStatefulProperties() override;

  /// Mechanical displacement strain increment in beam local coordinate system
  const MaterialProperty<RealVectorValue> & _disp_strain_increment;

  /// Mechanical rotational strain increment in beam local coordinate system
  const MaterialProperty<RealVectorValue> & _rot_strain_increment;

  /// Material stiffness vector that relates displacement strain increment to force increment
  const MaterialProperty<RealVectorValue> & _material_stiffness;

  /// Material flexure vector that relates rotational strain increment to moment increment
  const MaterialProperty<RealVectorValue> & _material_flexure;

  /// Rotational transformation from global to current beam local coordinate system
  const MaterialProperty<RankTwoTensor> & _total_rotation;

  /// Current force vector in global coordinate system
  MaterialProperty<RealVectorValue> & _force;

  /// Current moment vector in global coordinate system
  MaterialProperty<RealVectorValue> & _moment;

  /// Old force vector in global coordinate system
  const MaterialProperty<RealVectorValue> & _force_old;

  /// Old force vector in global coordinate system
  const MaterialProperty<RealVectorValue> & _moment_old;

  ///  yield and hardening property input
  RealVectorValue _yield_force;
  RealVectorValue _yield_moments;
  Real _kinematic_hardening_coefficient;
  Real _isotropic_hardening_coefficient;
  const Real _kinematic_hardening_slope;
  const Real _isotropic_hardening_slope;
  const Real _hardening_constant;

  /// convergence tolerance
  Real _absolute_tolerance;
  Real _relative_tolerance;

  /// Hardening variables at current and previous time step
  MaterialProperty<RealVectorValue> & _iso_hardening_variable_force;
  const MaterialProperty<RealVectorValue> & _iso_hardening_variable_force_old;
  MaterialProperty<RealVectorValue> & _iso_hardening_variable_moment;
  const MaterialProperty<RealVectorValue> & _iso_hardening_variable_moment_old;
  MaterialProperty<RealVectorValue> & _kin_hardening_variable_force;
  const MaterialProperty<RealVectorValue> & _kin_hardening_variable_force_old;
  MaterialProperty<RealVectorValue> & _kin_hardening_variable_moment;
  const MaterialProperty<RealVectorValue> & _kin_hardening_variable_moment_old;

  ///Plastic strains
  MaterialProperty<RealVectorValue> & _plastic_strain_translational;
  const MaterialProperty<RealVectorValue> & _plastic_strain_translational_old;
  MaterialProperty<RealVectorValue> & _plastic_strain_rotational;
  const MaterialProperty<RealVectorValue> & _plastic_strain_rotational_old;



  /// maximum no. of iterations
  const unsigned int _max_its;
};
