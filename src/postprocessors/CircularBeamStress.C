//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CircularBeamStress.h"

// MOOSE includes
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"

#include "libmesh/system.h"
#include "libmesh/string_to_enum.h"

registerMooseObject("MooseApp", CircularBeamStress);

defineLegacyParams(CircularBeamStress);

InputParameters
CircularBeamStress::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  MooseEnum stress_components("11=11 12 13");

  params.addRequiredParam<MooseEnum>(
      "stress_component",stress_components, "The component of the beam stress desired.");
  params.addRequiredParam<Point>("point",
                                 "The physical point where the solution will be evaluated.");
  params.addRequiredParam<Real>("radius",
                                "Radius of the section.");
  params.addParam<Real>("r_location",
                         0.0, "Relative location along the radius where the stress output is desired. (0 for center, 1 for edge)");
  params.addParam<Real>("theta",
                         0.0, "Location along the angular direction where the stress output is desired. Valid inputs range from 0 to 360");
  params.addClassDescription("Computes the value of beam stress component at a specified location");
  return params;
}

CircularBeamStress::CircularBeamStress(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _system(_subproblem.getSystem("forces_x")),
    _stress_component(getParam<MooseEnum>("stress_component")),
    _point(getParam<Point>("point")),
    _radius(getParam<Real>("radius")),
    _R(getParam<Real>("r_location")),
    _theta(getParam<Real>("theta")),
    _force_x(0),
    _force_y(0),
    _force_z(0),
    _moment_x(0),
    _moment_y(0),
    _moment_z(0),
    _value(0)
{
}

void
CircularBeamStress::execute()
{
  unsigned int _var_num1 = _subproblem
                  .getVariable(_tid,
                               "forces_x",
                               Moose::VarKindType::VAR_ANY,
                               Moose::VarFieldType::VAR_FIELD_STANDARD)
                  .number();

  _force_x = _system.point_value(_var_num1, _point, false);

  unsigned int _var_num2 = _subproblem
                  .getVariable(_tid,
                               "forces_y",
                               Moose::VarKindType::VAR_ANY,
                               Moose::VarFieldType::VAR_FIELD_STANDARD)
                  .number();

  _force_y = _system.point_value(_var_num2, _point, false);

  unsigned int _var_num3 = _subproblem
                  .getVariable(_tid,
                               "forces_z",
                               Moose::VarKindType::VAR_ANY,
                               Moose::VarFieldType::VAR_FIELD_STANDARD)
                  .number();

  _force_z = _system.point_value(_var_num3, _point, false);

  unsigned int _var_num4 = _subproblem
                  .getVariable(_tid,
                               "moments_x",
                               Moose::VarKindType::VAR_ANY,
                               Moose::VarFieldType::VAR_FIELD_STANDARD)
                  .number();

  _moment_x = _system.point_value(_var_num4, _point, false);

  unsigned int _var_num5 = _subproblem
                  .getVariable(_tid,
                               "moments_y",
                               Moose::VarKindType::VAR_ANY,
                               Moose::VarFieldType::VAR_FIELD_STANDARD)
                  .number();

  _moment_y = _system.point_value(_var_num5, _point, false);

  unsigned int _var_num6 = _subproblem
                  .getVariable(_tid,
                               "moments_z",
                               Moose::VarKindType::VAR_ANY,
                               Moose::VarFieldType::VAR_FIELD_STANDARD)
                  .number();

  _moment_z = _system.point_value(_var_num6, _point, false);


  /**
   * If we get exactly zero, we don't know if the locator couldn't find an element, or
   * if the solution is truly zero, more checking is needed.
   */
  if (MooseUtils::absoluteFuzzyEqual(_force_x, 0.0))
  {
    auto pl = _subproblem.mesh().getPointLocator();
    pl->enable_out_of_mesh_mode();

    auto * elem = (*pl)(_point);
    auto elem_id = elem ? elem->id() : DofObject::invalid_id;
    gatherMin(elem_id);

    if (elem_id == DofObject::invalid_id)
      mooseError("No element located at ", _point, " in CircularBeamStress Postprocessor named: ", name());
  }
}

Real
CircularBeamStress::getValue()
{
  Real _PI = 2 * std::acos(0.0);
  Real area = _PI * _radius * _radius;
  Real _y = _R * _radius * std::sin(_theta * _PI/180);
  Real _z = _R * _radius * std::cos(_theta * _PI/180);

  switch (_stress_component)
   {
    case 11:
      _value = _force_x/(area) + 4 * _moment_z * _y/(_PI * Utility::pow<4>(_radius))  +
             4 * _moment_y * _z/(_PI * Utility::pow<4>(_radius));
      break;
    case 12:
      _value = 4 * _force_y * (_radius * _radius - _y * _y)/(3 * _PI * Utility::pow<4>(_radius)) ;
      break;
    case 13:
      _value = 4 * _force_z * (_radius * _radius - _z * _z)/(3 * _PI * Utility::pow<4>(_radius)) ;
      break;
   }
  return _value;
}
