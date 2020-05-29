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

  std::cout<<" old force = "<<_force_old[_qp]<<"\n";

  _force[_qp] = _total_rotation[0].transpose() * force_increment + _force_old[_qp];


  // moment = R^T * _material_flexure * rotation_increment + moment_old
  RealVectorValue moment_increment;
  moment_increment(0) = _material_flexure[_qp](0) * _rot_strain_increment[_qp](0);
  moment_increment(1) = _material_flexure[_qp](1) * _rot_strain_increment[_qp](1);
  moment_increment(2) = _material_flexure[_qp](2) * _rot_strain_increment[_qp](2);

  std::cout<<" old moment = "<<_moment_old[_qp]<<"\n";

  _moment[_qp] = _total_rotation[0].transpose() * moment_increment + _moment_old[_qp];

  RealVectorValue trial_force = _force[_qp];
  RealVectorValue trial_moment = _moment[_qp];

  std::cout<<"yield force = "<<_yield_force<<"\n";
  std::cout<<"trial force = "<<trial_force<<std::endl;
  std::cout<<"trial moment = "<<trial_moment<<std::endl;

  Real yield_condition = Utility::pow<2>(trial_force(0)/_yield_force(0)) +
                         Utility::pow<2>(trial_force(1)/_yield_force(1)) +
                        Utility::pow<2>(trial_force(2)/_yield_force(2)) +
                        Utility::pow<2>(trial_moment(0)/_yield_moments(0)) +
                        Utility::pow<2>(trial_moment(1)/_yield_moments(1)) +
                        Utility::pow<2>(trial_moment(2)/_yield_moments(2)) -
                        1.0;

  std::cout<<"yield_condition = "<<yield_condition<<std::endl;

  if (yield_condition > 0.0)
  {
    std::cout<<"\n true \n";
    RealVectorValue dphidF;
    RealVectorValue dphidM;
    RealVectorValue d2phidF;
    RealVectorValue d2phidM;
    RealVectorValue Q_F;
    RealVectorValue Q_M;
    RealVectorValue F_hat;
    RealVectorValue M_hat;
    RealVectorValue residual_force_vector;
    RealVectorValue residual_moment_vector;

    Real denom = 0;
    for (unsigned int i =0; i<3; ++i)
    {
      dphidF(i) = 2 * (trial_force(i)/_yield_force(i));
      dphidM(i) = 2 * (trial_moment(i)/_yield_moments(i));
      denom += dphidF(i) * _material_stiffness[_qp](i) * dphidF(i);
      denom += dphidM(i) * _material_flexure[_qp](i) * dphidM(i);
    }

    std::cout<<"dphidF = "<<dphidF<<std::endl;
    std::cout<<"dphidM = "<<dphidM<<std::endl;
    std::cout<<"denom = "<<denom<<std::endl;




    Real lambda = yield_condition/denom;

    std::cout<<"lambda = "<<lambda<<std::endl;


    for (unsigned int i =0; i<3; ++i)
    {
      F_hat(i) = trial_force(i) - lambda * _material_stiffness[_qp](i) * dphidF(i);
      M_hat(i) = trial_moment(i) - lambda * _material_flexure[_qp](i) * dphidM(i);
      dphidF(i) = 2 * (F_hat(i)/_yield_force(i));
      dphidM(i) = 2 * (M_hat(i)/_yield_moments(i));
    }

    std::cout<<"dphidF = "<<dphidF<<std::endl;
    std::cout<<"dphidM = "<<dphidM<<std::endl;
    std::cout<<"F_hat = "<<F_hat<<std::endl;
    std::cout<<"M_hat= "<<M_hat<<std::endl;


    yield_condition = Utility::pow<2>(F_hat(0)/_yield_force(0)) +
                      Utility::pow<2>(F_hat(1)/_yield_force(1)) +
                      Utility::pow<2>(F_hat(2)/_yield_force(2)) +
                      Utility::pow<2>(M_hat(0)/_yield_moments(0)) +
                      Utility::pow<2>(M_hat(1)/_yield_moments(1)) +
                      Utility::pow<2>(M_hat(2)/_yield_moments(2)) -
                      1.0;

    RealVectorValue F = F_hat;
    RealVectorValue M = M_hat;

    for (unsigned int i =0; i<3; ++i)
    {
      residual_force_vector(i) = F(i) - (trial_force(i) - lambda * _material_stiffness[_qp](i) * dphidF(i));
      residual_moment_vector(i) = M(i) - (trial_moment(i) - lambda * _material_flexure[_qp](i) * dphidM(i));
    }

    std::cout<<"res force = "<<residual_force_vector<<std::endl;
    std::cout<<"res moment = "<<residual_moment_vector<<std::endl;


    Real iteration = 0;

    while(std::abs(yield_condition) > _absolute_tolerance)
    // while (iteration<2)
    {
      ++iteration;
      yield_condition = Utility::pow<2>(F(0)/_yield_force(0)) +
                        Utility::pow<2>(F(1)/_yield_force(1)) +
                        Utility::pow<2>(F(2)/_yield_force(2)) +
                        Utility::pow<2>(M(0)/_yield_moments(0)) +
                        Utility::pow<2>(M(1)/_yield_moments(1)) +
                        Utility::pow<2>(M(2)/_yield_moments(2)) -
                        1.0;

      Real numer = 0;
      Real denomr = 0;
      for (unsigned int i =0; i<3; ++i)
      {
        dphidF(i) = 2 * (F(i)/_yield_force(i));
        dphidM(i) = 2 * (M(i)/_yield_moments(i));
        d2phidF(i) = 2/_yield_force(i);
        d2phidM(i) = 2/_yield_moments(i);
        residual_force_vector(i) = F(i) - trial_force(i) + lambda * _material_stiffness[_qp](i) * dphidF(i);
        residual_moment_vector(i) = M(i) - trial_moment(i) + lambda * _material_flexure[_qp](i) * dphidM(i);
        Q_F(i) = 1.0 + lambda * _material_stiffness[_qp](i) * d2phidF(i);
        Q_M(i) = 1.0 + lambda * _material_flexure[_qp](i) * d2phidM(i);
        numer += dphidF(i) * (1/Q_F(i)) * residual_force_vector(i);
        numer += dphidM(i) * (1/Q_M(i)) * residual_moment_vector(i);
        denomr += dphidF(i) * (1/Q_F(i)) * _material_stiffness[_qp](i) * dphidF(i);
        denomr += dphidM(i) * (1/Q_M(i)) * _material_flexure[_qp](i) * dphidM(i);
      }

      Real lambda_dot;
      lambda_dot = (yield_condition - numer)/denomr;

      for (unsigned int i =0; i<3; ++i)
      {
        force_increment(i) = -(1/Q_F(i)) * (residual_force_vector(i) + lambda_dot * _material_stiffness[_qp](i) * dphidF(i));
        moment_increment(i) = -(1/Q_M(i)) * (residual_moment_vector(i) + lambda_dot * _material_flexure[_qp](i) * dphidM(i));
      }

      F += force_increment;
      M += moment_increment;
      lambda += lambda_dot;

      // std::cout<<"iteration = "<<iteration<<"\n";
      // std::cout<<"F = "<<F<<"\n";
      // std::cout<<"M = "<<M<<"\n\n";

    }
    trial_force = F;
    trial_moment = M;

    std::cout<<"number of plastic iterations = "<<iteration<<std::endl;
  }

  _force[_qp] = trial_force;
  _moment[_qp] = trial_moment;

  std::cout<<"yeild condition after convergence "<<yield_condition<<std::endl;
  std::cout<<"force from CBR"<<_qp<<" = "<<_force[_qp];
  std::cout<<"moment from CBR"<<_qp<<" = "<<_moment[_qp];
}
