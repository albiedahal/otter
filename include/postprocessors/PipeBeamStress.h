//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"
#include "MooseEnum.h"

// Forward Declarations
class PipeBeamStress;

template <>
InputParameters validParams<PipeBeamStress>();

/**
 * Compute the value of a variable at a specified location.
 *
 * Warning: This postprocessor may result in undefined behavior if utilized with
 * non-continuous elements and the point being located lies on an element boundary.
 */
class PipeBeamStress : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  PipeBeamStress(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual Real getValue() override;

protected:
  /// A reference to the system containing the variable
  const System & _system;

  MooseEnum _stress_component;

  /// The point to locate
  const Point & _point;
  const Real & _ro;
  const Real & _thickness;

  const Real & _R;
  const Real & _theta;

  Real _force_x;
  Real _force_y;
  Real _force_z;
  Real _moment_x;
  Real _moment_y;
  Real _moment_z;


  /// The value of the variable at the desired location
  Real _value;
};
