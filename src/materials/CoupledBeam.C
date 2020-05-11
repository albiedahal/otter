//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledBeam.h"

registerMooseObject("TensorMechanicsApp", CoupledBeam);

defineLegacyParams(CoupledBeam);

InputParameters
CoupledBeam::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute forces and moments using elasticity");
  params.addRequiredParam<RealVectorValue>("yield_force",
                             "Yield force after which plastic strain starts accumulating");
  params.addRequiredParam<RealVectorValue>("yield_moments",
                             "Yield moments after which plastic strain starts accumulating");
  params.addParam<Real>("hardening_constant", 0.0, "Hardening slope");
  params.addParam<Real>(
     "absolute_tolerance", 1e-10, "Absolute convergence tolerance for Newton iteration");
  params.addParam<Real>(
     "relative_tolerance", 1e-8, "Relative convergence tolerance for Newton iteration");
  return params;
}

CoupledBeam::CoupledBeam(const InputParameters & parameters)
  : Material(parameters),
    _disp_strain_increment(
        getMaterialPropertyByName<RealVectorValue>("mech_disp_strain_increment")),
    _rot_strain_increment(getMaterialPropertyByName<RealVectorValue>("mech_rot_strain_increment")),
    _material_stiffness(getMaterialPropertyByName<RealVectorValue>("material_stiffness")),
    _material_flexure(getMaterialPropertyByName<RealVectorValue>("material_flexure")),
    _total_rotation(getMaterialPropertyByName<RankTwoTensor>("total_rotation")),
    _force(declareProperty<RealVectorValue>("forces")),
    _moment(declareProperty<RealVectorValue>("moments")),
    _force_old(getMaterialPropertyOld<RealVectorValue>("forces")),
    _moment_old(getMaterialPropertyOld<RealVectorValue>("moments")),
    _yield_force(getParam<RealVectorValue>("yield_force")),
    _yield_moments(getParam<RealVectorValue>("yield_moments")),
    _hardening_constant(getParam<Real>("hardening_constant")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _plastic_deformation(declareProperty<RealVectorValue>("_plastic_deformation")),
    _plastic_deformation_old(getMaterialPropertyOld<RealVectorValue>("_plastic_deformation")),
    _plastic_rotation(declareProperty<RealVectorValue>("_plastic_rotation")),
    _plastic_rotation_old(getMaterialPropertyOld<RealVectorValue>("_plastic_rotation")),
    _hardening_variable_force(declareProperty<RealVectorValue>("hardening_variable_force")),
    _hardening_variable_force_old(getMaterialPropertyOld<RealVectorValue>("hardening_variable_force")),
    _hardening_variable_moment(declareProperty<RealVectorValue>("hardening_variable_moment")),
    _hardening_variable_moment_old(getMaterialPropertyOld<RealVectorValue>("hardening_variable_moment"))
{
}

void
CoupledBeam::initQpStatefulProperties()
{
  _force[_qp].zero();
  _moment[_qp].zero();
  _plastic_deformation[_qp].zero();
  _plastic_rotation[_qp].zero();
  _hardening_variable_force[_qp] = 0.0;
  _hardening_variable_moment[_qp] = 0.0;
}

void
CoupledBeam::computeQpProperties()
{
  // force = R^T * _material_stiffness * strain_increment + force_old
  RealVectorValue force_increment;
  force_increment(0) = _material_stiffness[_qp](0) * _disp_strain_increment[_qp](0);
  force_increment(1) = _material_stiffness[_qp](1) * _disp_strain_increment[_qp](1);
  force_increment(2) = _material_stiffness[_qp](2) * _disp_strain_increment[_qp](2);

  RealVectorValue trial_force = _total_rotation[0].transpose() * force_increment + _force_old[_qp];


  // moment = R^T * _material_flexure * rotation_increment + moment_old
  RealVectorValue moment_increment;
  moment_increment(0) = _material_flexure[_qp](0) * _rot_strain_increment[_qp](0);
  moment_increment(1) = _material_flexure[_qp](1) * _rot_strain_increment[_qp](1);
  moment_increment(2) = _material_flexure[_qp](2) * _rot_strain_increment[_qp](2);

  RealVectorValue trial_moment = _total_rotation[0].transpose() * moment_increment + _moment_old[_qp];

  _hardening_variable_force[_qp] = _hardening_variable_force_old[_qp];
  _hardening_variable_moment[_qp] = _hardening_variable_moment_old[_qp];
  _plastic_deformation[_qp] = _plastic_deformation_old[_qp];
  _plastic_rotation[_qp] = _plastic_rotation_old[_qp];

  Real yield_condition = Utility::pow<2>(trial_force(0)/(_hardening_variable_force[_qp](0) + _yield_force(0))) +
                         Utility::pow<2>(trial_force(1)/(_hardening_variable_force[_qp](1) + _yield_force(1))) +
                        Utility::pow<2>(trial_force(2)/(_hardening_variable_force[_qp](2) + _yield_force(2))) +
                        Utility::pow<2>(trial_moment(0)/(_hardening_variable_moment[_qp](0) + _yield_moments(0))) +
                        Utility::pow<2>(trial_moment(1)/(_hardening_variable_moment[_qp](1) + _yield_moments(1))) +
                        Utility::pow<2>(trial_moment(2)/(_hardening_variable_moment[_qp](2) + _yield_moments(2))) -
                        1.0;
  Real iteration = 0;
  RealVectorValue plastic_deformation_increment = 0.0;
  RealVectorValue elastic_deformation_increment = _plastic_deformation[_qp];
  RealVectorValue plastic_rotation_increment = 0.0;
  RealVectorValue elastic_rotation_increment = _plastic_rotation[_qp];

  if (yield_condition > 0.0)
  {

  }
}
