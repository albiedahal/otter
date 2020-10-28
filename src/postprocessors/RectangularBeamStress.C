//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RectangularBeamStress.h"

// MOOSE includes
#include "Function.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SubProblem.h"

#include "libmesh/system.h"
#include "libmesh/string_to_enum.h"

registerMooseObject("MooseApp", RectangularBeamStress);

defineLegacyParams(RectangularBeamStress);

InputParameters
RectangularBeamStress::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  MooseEnum stress_components("11=11 12 13");

  params.addRequiredParam<MooseEnum>(
      "stress_component",stress_components, "The component of the beam stress desired.");
  params.addRequiredParam<Point>("point",
                                 "The physical point where the solution will be evaluated.");
  params.addRequiredParam<Real>("depth",
                                "The depth of the section.");
  params.addRequiredParam<Real>("width",
                                "The width of the section.");
  params.addParam<Real>("y_location",
                         0.0, "Relative location along the section depth where the stress output is desired. (0 for bottom, 1 for top)");
  params.addParam<Real>("z_location",
                         0.0, "Relative location along the section width where the stress output is desired. (0 for left, 1 for right)");
  params.addClassDescription("Computes the value of beam stress component at a specified location");
  return params;
}

RectangularBeamStress::RectangularBeamStress(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _system(_subproblem.getSystem("forces_x")),
    _stress_component(getParam<MooseEnum>("stress_component")),
    _point(getParam<Point>("point")),
    _depth(getParam<Real>("depth")),
    _width(getParam<Real>("width")),
    _y(getParam<Real>("y_location")),
    _z(getParam<Real>("z_location")),
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
RectangularBeamStress::execute()
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
      mooseError("No element located at ", _point, " in RectangularBeamStress Postprocessor named: ", name());
  }
}

Real
RectangularBeamStress::getValue()
{


  switch (_stress_component)
   {
    case 11:
      _value = _force_x/(_depth * _width) + 12 * _moment_z * (_y - 0.5) * _depth/(_width * Utility::pow<3>(_depth))  +
             12 * _moment_y * (_z - 0.5) * _width/(_depth * Utility::pow<3>(_width));
      break;
    case 12:
      _value = 6 * _force_y * _depth * _depth * (0.25 - (_y - 0.5) * (_y - 0.5))/(_width * Utility::pow<3>(_depth)) ;
      break;
    case 13:
      _value = 6 * _force_z * _width * _width * (0.25 - (_z - 0.5) * (_z - 0.5))/(_depth * Utility::pow<3>(_width)) ;
      break;
   }
  return _value;
}
