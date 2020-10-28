//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PipeBeamStress.h"

// MOOSE includes
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"

#include "libmesh/system.h"
#include "libmesh/string_to_enum.h"

registerMooseObject("MooseApp", PipeBeamStress);

defineLegacyParams(PipeBeamStress);

InputParameters
PipeBeamStress::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  MooseEnum stress_components("11=11 12 13");

  params.addRequiredParam<MooseEnum>(
      "stress_component",stress_components, "The component of the beam stress desired.");
  params.addRequiredParam<Point>("point",
                                 "The physical point where the solution will be evaluated.");
  params.addRequiredParam<Real>("radius",
                                "Outer radius of the section.");
  params.addRequiredParam<Real>("thickness",
                                "Thickness of the section.");
  params.addParam<Real>("r_location",
                         0.0, "Relative location along the thickness where the stress output is desired. (0 for inner edge, 1 for outer edge)");
  params.addParam<Real>("theta",
                         0.0, "Location along the angular direction where the stress output is desired. Valid inputs range from 0 to 360");
  params.addClassDescription("Computes the value of beam stress component at a specified location");
  return params;
}

PipeBeamStress::PipeBeamStress(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _system(_subproblem.getSystem("forces_x")),
    _stress_component(getParam<MooseEnum>("stress_component")),
    _point(getParam<Point>("point")),
    _ro(getParam<Real>("radius")),
    _thickness(getParam<Real>("thickness")),
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
PipeBeamStress::execute()
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
      mooseError("No element located at ", _point, " in PipeBeamStress Postprocessor named: ", name());
  }
}

Real
PipeBeamStress::getValue()
{
  Real _PI = 2 * std::acos(0.0);
  Real _ri = _ro - _thickness;
  Real area = _PI * (_ro * _ro - _ri * _ri);
  Real _y = (_R * _thickness + _ri) * std::sin(_theta * _PI/180);
  Real _z = (_R * _thickness + _ri) * std::cos(_theta * _PI/180);
  Real Q;
  Real b;

  switch (_stress_component)
   {
    case 11:
      _value = _force_x/(area) + 4 * _moment_z * _y/(_PI * (Utility::pow<4>(_ro) - Utility::pow<4>(_ri)))  +
             4 * _moment_y * _z/(_PI * (Utility::pow<4>(_ro) - Utility::pow<4>(_ri)));
      break;
    case 12:
      if(_y<_ri)
      {
        Q = 2 * (std::pow(_ro * _ro - _y * _y,1.5) - std::pow(_ri * _ri - _y * _y,1.5))/3;
        b = 2 * (std::sqrt(_ro * _ro - _y * _y) - std::sqrt(_ri * _ri - _y * _y));
      }
      else
      {
        Q = 2 * (std::pow(_ro * _ro - _y * _y,1.5))/3;
        b = 2 * (std::sqrt(_ro * _ro - _y * _y));
      }
      _value = 4 * _force_y * Q /(_PI *(Utility::pow<4>(_ro) - Utility::pow<4>(_ri)) * b) ;
      break;
    case 13:
    if(_z<_ri)
    {
      Q = 2 * (std::pow(_ro * _ro - _z * _z,1.5) - std::pow(_ri * _ri - _z * _z,1.5))/3;
      b = 2 * (std::sqrt(_ro * _ro - _z * _z) - std::sqrt(_ri * _ri - _z * _z));
    }
    else
    {
      Q = 2 * (std::pow(_ro * _ro - _z * _z,1.5))/3;
      b = 2 * (std::sqrt(_ro * _ro - _z * _z));
    }
    _value = 4 * _force_z * Q /(_PI *(Utility::pow<4>(_ro) - Utility::pow<4>(_ri)) * b) ;
      break;
   }
  return _value;
}
