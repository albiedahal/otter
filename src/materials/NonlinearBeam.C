//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonlinearBeam.h"

registerMooseObject("TensorMechanicsApp", NonlinearBeam);

defineLegacyParams(NonlinearBeam);

InputParameters
NonlinearBeam::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute forces and moments using elasticity");
  params.addRequiredParam<RealVectorValue>("yield_force",
                             "Yield force after which plastic strain starts accumulating");
  params.addRequiredParam<RealVectorValue>("yield_moments",
                             "Yield moments after which plastic strain starts accumulating");
  params.addParam<Real>("kinematic_hardening_coefficient", 0.0, "Kinematic Hardening coefficient");
  params.addParam<Real>("isotropic_hardening_coefficient", 0.0, "Isotropic Hardening coefficient");
  params.addParam<Real>("kinematic_hardening_slope",0.0,"Kinematic hardening slope");
  params.addParam<Real>("isotropic_hardening_slope",0.0,"Isotropic hardening slope");
  params.addParam<Real>("hardening_constant", 1.0, "Hardening constant (c) value between [0,1]. If c=1 it is isotropic hardening_constant"
                        "if c = 0, it is kinematic hardening and if c (0,1), it is mixed hardening");
  params.addParam<Real>(
     "absolute_tolerance", 1e-10, "Absolute convergence tolerance for Newton iteration");
  params.addParam<Real>(
     "relative_tolerance", 1e-8, "Relative convergence tolerance for Newton iteration");
  return params;
}

NonlinearBeam::NonlinearBeam(const InputParameters & parameters)
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
    _kinematic_hardening_coefficient(getParam<Real>("kinematic_hardening_coefficient")),
    _isotropic_hardening_coefficient(getParam<Real>("isotropic_hardening_coefficient")),
    _kinematic_hardening_slope(getParam<Real>("kinematic_hardening_slope")),
    _isotropic_hardening_slope(getParam<Real>("isotropic_hardening_slope")),
    _hardening_constant(getParam<Real>("hardening_constant")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _iso_hardening_variable_force(declareProperty<RealVectorValue>("isotropic hardening_variable_force")),
    _iso_hardening_variable_force_old(getMaterialPropertyOld<RealVectorValue>("isotropic hardening_variable_force")),
    _iso_hardening_variable_moment(declareProperty<RealVectorValue>("isotropic hardening_variable_moment")),
    _iso_hardening_variable_moment_old(getMaterialPropertyOld<RealVectorValue>("isotropic hardening_variable_moment")),
    _kin_hardening_variable_force(declareProperty<RealVectorValue>("kinematic hardening_variable_force")),
    _kin_hardening_variable_force_old(getMaterialPropertyOld<RealVectorValue>("kinematic hardening_variable_force")),
    _kin_hardening_variable_moment(declareProperty<RealVectorValue>("kinematic hardening_variable_moment")),
    _kin_hardening_variable_moment_old(getMaterialPropertyOld<RealVectorValue>("kinematic hardening_variable_moment")),
    _max_its(1000)

{
  if(parameters.isParamSetByUser("kinematic_hardening_slope") && parameters.isParamSetByUser("kinematic_hardening_coefficient"))
    mooseError("NonlinearBeam: Only the kinematic_hardening_slope or only the kinematic_hardening_coefficient can be defined but not both");
  if(parameters.isParamSetByUser("isotropic_hardening_slope") && parameters.isParamSetByUser("isotropic_hardening_coefficient"))
    mooseError("NonlinearBeam: Only the isotropic_hardening_slope or only the isotropic_hardening_coefficient can be defined but not both");
}

void
NonlinearBeam::initQpStatefulProperties()
{
  _force[_qp].zero();
  _moment[_qp].zero();
  _iso_hardening_variable_force[_qp].zero();
  _iso_hardening_variable_moment[_qp].zero();
  _kin_hardening_variable_force[_qp].zero();
  _kin_hardening_variable_moment[_qp].zero();
}

void
NonlinearBeam::computeQpProperties()
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

  _iso_hardening_variable_force[_qp] = _iso_hardening_variable_force_old[_qp];
  _iso_hardening_variable_moment[_qp] = _iso_hardening_variable_moment_old[_qp];
  _kin_hardening_variable_force[_qp] = _kin_hardening_variable_force_old[_qp];
  _kin_hardening_variable_moment[_qp] = _kin_hardening_variable_moment_old[_qp];

  if(_kinematic_hardening_slope)
    _kinematic_hardening_coefficient = _kinematic_hardening_slope/(1 - _kinematic_hardening_slope);

  if(_isotropic_hardening_slope)
    _isotropic_hardening_coefficient = _isotropic_hardening_slope/(1 - _isotropic_hardening_slope);

  // std::cout<<"yield force = "<<_yield_force<<"\n";
  std::cout<<"trial force = "<<trial_force<<std::endl;
  std::cout<<"trial moment = "<<trial_moment<<std::endl;
  // std::cout<<"iso har var for = "<<_iso_hardening_variable_force[_qp]<<"\n";
  std::cout<<"iso har var mom = "<<_iso_hardening_variable_moment[_qp]<<"\n";


  Real yield_condition = Utility::pow<2>((trial_force(0) - _kin_hardening_variable_force[_qp](0))/(_yield_force(0)+ _iso_hardening_variable_force[_qp](0))) +
                         // Utility::pow<2>((trial_force(1) - _kin_hardening_variable_force[_qp](1))/(_yield_force(1) + _iso_hardening_variable_force[_qp](1))) +
                        // Utility::pow<2>((trial_force(2) - _kin_hardening_variable_force[_qp](2))/(_yield_force(2)+ _iso_hardening_variable_force[_qp](2))) +
                        Utility::pow<2>((trial_moment(0) - _kin_hardening_variable_moment[_qp](0))/(_yield_moments(0) + _iso_hardening_variable_moment[_qp](0))) +
                        Utility::pow<2>((trial_moment(1) - _kin_hardening_variable_moment[_qp](1))/(_yield_moments(1) + _iso_hardening_variable_moment[_qp](1))) +
                        Utility::pow<2>((trial_moment(2) - _kin_hardening_variable_moment[_qp](2))/(_yield_moments(2) + _iso_hardening_variable_moment[_qp](2))) -
                        1.0;

  std::cout<<"yield_condition = "<<yield_condition<<std::endl;

  if (yield_condition > _absolute_tolerance)
  {
    RealVectorValue dphidF;
    RealVectorValue dphidM;
    RealVectorValue d2phidF;
    RealVectorValue d2phidM;
    RealVectorValue dphidFy;
    RealVectorValue dphidMy;
    RealVectorValue dphidFa;
    RealVectorValue dphidMa;
    RealVectorValue Q_F;
    RealVectorValue Q_M;
    RealVectorValue F_hat;
    RealVectorValue M_hat;
    RealVectorValue residual_force_vector;
    RealVectorValue residual_moment_vector;


    Real denom = 0;
    Real AI = 0;
    Real AK = 0;
    dphidF(0) = 2 * ((trial_force(0) - _kin_hardening_variable_force[_qp](0))/Utility::pow<2>(_yield_force(0)+ _iso_hardening_variable_force[_qp](0)));
    dphidF(1) = dphidF(2) = 0;
    dphidFy(0) = -2 * (Utility::pow<2>(trial_force(0) - _kin_hardening_variable_force[_qp](0))/Utility::pow<3>(_yield_force(0)+ _iso_hardening_variable_force[_qp](0)));
    dphidFy(1) = dphidFy(2) = 0;
    dphidFa(0) = -2 * (trial_force(0) - _kin_hardening_variable_force[_qp](0))/Utility::pow<2>(_yield_force(0) + _iso_hardening_variable_force[_qp](0));
    dphidFa(1) = dphidFa(2) = 0;
    for (unsigned int i =0; i<3; ++i)
    {
      // dphidF(i) = 2 * ((trial_force(i) - _kin_hardening_variable_force[_qp](i))/Utility::pow<2>(_yield_force(i)+ _iso_hardening_variable_force[_qp](i)));
      dphidM(i) = 2 * ((trial_moment(i) - _kin_hardening_variable_moment[_qp](i))/Utility::pow<2>(_yield_moments(i)+ _iso_hardening_variable_moment[_qp](i)));
      // dphidFy(i) = -2 * (Utility::pow<2>(trial_force(i) - _kin_hardening_variable_force[_qp](i))/Utility::pow<3>(_yield_force(i)+ _iso_hardening_variable_force[_qp](i)));
      dphidMy(i) = -2 * (Utility::pow<2>(trial_moment(i) - _kin_hardening_variable_moment[_qp](i))/Utility::pow<3>(_yield_moments(i)+ _iso_hardening_variable_moment[_qp](i)));
      // dphidFa(i) = -2 * (trial_force(i) - _kin_hardening_variable_force[_qp](i))/Utility::pow<2>(_yield_force(i) + _iso_hardening_variable_force[_qp](i));
      dphidMa(i) = -2 * (trial_moment(i) - _kin_hardening_variable_moment[_qp](i))/Utility::pow<2>(_yield_moments(i) + _iso_hardening_variable_moment[_qp](i));
      denom += dphidF(i) * _material_stiffness[_qp](i) * dphidF(i);
      denom += dphidM(i) * _material_flexure[_qp](i) * dphidM(i);
      AI += dphidF(i) * _material_stiffness[_qp](i) * dphidFy(i);
      AI += dphidM(i) * _material_flexure[_qp](i) * dphidMy(i);
      AK += dphidF(i) * _material_stiffness[_qp](i) * dphidFa(i);
      AK += dphidM(i) * _material_flexure[_qp](i) * dphidMa(i);
    }

    // std::cout<<"dphidF = "<<dphidF<<std::endl;
    // std::cout<<"dphidM = "<<dphidM<<std::endl;
    // std::cout<<"dphidMy = "<<dphidMy<<std::endl;
    // std::cout<<"denom = "<<denom<<std::endl;
    // std::cout<<"A = "<<AI<<std::endl;




    Real lambda = yield_condition/(denom - _hardening_constant * _isotropic_hardening_coefficient * AI - (1 - _hardening_constant) * _kinematic_hardening_coefficient * AK);
    // lambda *= MathUtils::sign(trial_force(1));

    std::cout<<"lambda = "<<lambda<<std::endl;

    for (unsigned int i =0; i<3; ++i)
    {
      _iso_hardening_variable_force[_qp](i) = _iso_hardening_variable_force_old[_qp](i) + _hardening_constant * _isotropic_hardening_coefficient * _material_stiffness[_qp](i) * std::abs(lambda * dphidF(i));
      _iso_hardening_variable_moment[_qp](i) = _iso_hardening_variable_moment_old[_qp](i) + _hardening_constant * _isotropic_hardening_coefficient * _material_flexure[_qp](i) * std::abs(lambda * dphidM(i));
      _kin_hardening_variable_force[_qp](i) = _kin_hardening_variable_force_old[_qp](i) + (1 - _hardening_constant) * _kinematic_hardening_coefficient * _material_stiffness[_qp](i) * lambda * dphidF(i);
      _kin_hardening_variable_moment[_qp](i) = _kin_hardening_variable_moment_old[_qp](i) + (1 - _hardening_constant) * _kinematic_hardening_coefficient * _material_flexure[_qp](i) * lambda * dphidM(i);
      F_hat(i) = trial_force(i) - lambda * _material_stiffness[_qp](i) * dphidF(i);
      M_hat(i) = trial_moment(i) - lambda * _material_flexure[_qp](i) * dphidM(i);
      // dphidF(i) = 2 * (F_hat(i)/Utility::pow<2>(_yield_force(i)+ _iso_hardening_variable_force[_qp](i)));
      dphidM(i) = 2 * ((M_hat(i) - _kin_hardening_variable_moment[_qp](i))/Utility::pow<2>(_yield_moments(i)+ _iso_hardening_variable_moment[_qp](i)));
    }
    dphidF(0) = 2 * ((F_hat(0) - _kin_hardening_variable_force[_qp](0))/Utility::pow<2>(_yield_force(0)+ _iso_hardening_variable_force[_qp](0)));

    // std::cout<<"dphidF = "<<dphidF<<std::endl;
    // std::cout<<"dphidM = "<<dphidM<<std::endl;
    std::cout<<"F_hat = "<<F_hat<<std::endl;
    std::cout<<"M_hat= "<<M_hat<<std::endl;
    std::cout<<"iso har var mom after lambda = "<<_iso_hardening_variable_moment[_qp]<<"\n";



    yield_condition = Utility::pow<2>((F_hat(0) - _kin_hardening_variable_force[_qp](0))/(_yield_force(0) + _iso_hardening_variable_force[_qp](0))) +
                      // Utility::pow<2>((F_hat(1) - _kin_hardening_variable_force[_qp](1))/(_yield_force(1) + _iso_hardening_variable_force[_qp](1))) +
                      // Utility::pow<2>((F_hat(2) - _kin_hardening_variable_force[_qp](2))/(_yield_force(2)+ _iso_hardening_variable_force[_qp](2))) +
                      Utility::pow<2>((M_hat(0) - _kin_hardening_variable_moment[_qp](0))/(_yield_moments(0)+ _iso_hardening_variable_moment[_qp](0))) +
                      Utility::pow<2>((M_hat(1) - _kin_hardening_variable_moment[_qp](1))/(_yield_moments(1)+ _iso_hardening_variable_moment[_qp](1))) +
                      Utility::pow<2>((M_hat(2) - _kin_hardening_variable_moment[_qp](2))/(_yield_moments(2) + _iso_hardening_variable_moment[_qp](2))) -
                      1.0;

    std::cout<<"yield condition after hat = "<<yield_condition<<"\n";

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
    // while (iteration<8)
    {
      yield_condition = Utility::pow<2>((F(0) - _kin_hardening_variable_force[_qp](0))/(_yield_force(0)+ _iso_hardening_variable_force[_qp](0))) +
                        // Utility::pow<2>((F(1) - _kin_hardening_variable_force[_qp](1))/(_yield_force(1)+ _iso_hardening_variable_force[_qp](1))) +
                        // Utility::pow<2>((F(2) - _kin_hardening_variable_force[_qp](2))/(_yield_force(2)+ _iso_hardening_variable_force[_qp](2))) +
                        Utility::pow<2>((M(0) - _kin_hardening_variable_moment[_qp](0))/(_yield_moments(0)+ _iso_hardening_variable_moment[_qp](0))) +
                        Utility::pow<2>((M(1) - _kin_hardening_variable_moment[_qp](1))/(_yield_moments(1)+ _iso_hardening_variable_moment[_qp](1))) +
                        Utility::pow<2>((M(2) - _kin_hardening_variable_moment[_qp](2))/(_yield_moments(2)+ _iso_hardening_variable_moment[_qp](2))) -
                        1.0;

      Real numer = 0;
      Real denomr = 0;
      Real BI = 0;
      Real BK = 0;
      dphidF(0) = 2 * ((F(0) - _kin_hardening_variable_force[_qp](0))/Utility::pow<2>(_yield_force(0)+ _iso_hardening_variable_force[_qp](0)));
      d2phidF(0) = 2/Utility::pow<2>(_yield_force(0) + _iso_hardening_variable_force[_qp](0));
      d2phidF(1) = d2phidF(2) = 0;
      dphidFy(0) = -2 * (Utility::pow<2>(F(0) - _kin_hardening_variable_force[_qp](0))/Utility::pow<3>(_yield_force(0)+ _iso_hardening_variable_force[_qp](0)));
      dphidFa(0) = -2 * (F(0) - _kin_hardening_variable_force[_qp](0))/Utility::pow<2>(_yield_force(0) + _iso_hardening_variable_force[_qp](0));

      for (unsigned int i =0; i<3; ++i)
      {
        // dphidF(i) = 2 * ((F(i) - _kin_hardening_variable_force[_qp](i))/Utility::pow<2>(_yield_force(i)+ _iso_hardening_variable_force[_qp](i)));
        dphidM(i) = 2 * ((M(i) - _kin_hardening_variable_moment[_qp](i))/Utility::pow<2>(_yield_moments(i)+ _iso_hardening_variable_moment[_qp](i)));
        // dphidFy(i) = -2 * (Utility::pow<2>(F(i) - _kin_hardening_variable_force[_qp](i))/Utility::pow<3>(_yield_force(i)+ _iso_hardening_variable_force[_qp](i)));
        dphidMy(i) = -2 * (Utility::pow<2>(M(i) - _kin_hardening_variable_moment[_qp](i))/Utility::pow<3>(_yield_moments(i)+ _iso_hardening_variable_moment[_qp](i)));
        // dphidFa(i) = -2 * (F(i) - _kin_hardening_variable_force[_qp](i))/Utility::pow<2>(_yield_force(i) + _iso_hardening_variable_force[_qp](i));
        dphidMa(i) = -2 * (M(i) - _kin_hardening_variable_moment[_qp](i))/Utility::pow<2>(_yield_moments(i) + _iso_hardening_variable_moment[_qp](i));
        // d2phidF(i) = 2/Utility::pow<2>(_yield_force(i) + _iso_hardening_variable_force[_qp](i));
        d2phidM(i) = 2/Utility::pow<2>(_yield_moments(i) + _iso_hardening_variable_moment[_qp](i));
        residual_force_vector(i) = F(i) - trial_force(i) + lambda * _material_stiffness[_qp](i) * dphidF(i);
        residual_moment_vector(i) = M(i) - trial_moment(i) + lambda * _material_flexure[_qp](i) * dphidM(i);
        Q_F(i) = 1.0 + lambda * _material_stiffness[_qp](i) * d2phidF(i);
        Q_M(i) = 1.0 + lambda * _material_flexure[_qp](i) * d2phidM(i);
        numer += dphidF(i) * (1/Q_F(i)) * residual_force_vector(i);
        numer += dphidM(i) * (1/Q_M(i)) * residual_moment_vector(i);
        denomr += dphidF(i) * (1/Q_F(i)) * _material_stiffness[_qp](i) * dphidF(i);
        denomr += dphidM(i) * (1/Q_M(i)) * _material_flexure[_qp](i) * dphidM(i);
        BI += dphidF(i) * _material_stiffness[_qp](i) * dphidFy(i);
        BI += dphidM(i) * _material_flexure[_qp](i) * dphidMy(i);
        BK += dphidF(i) * _material_stiffness[_qp](i) * dphidFa(i);
        BK += dphidM(i) * _material_flexure[_qp](i) * dphidMa(i);
      }

      Real lambda_dot;
      lambda_dot = (yield_condition - numer)/(denomr - _hardening_constant * _isotropic_hardening_coefficient * BI - (1 - _hardening_constant) * _kinematic_hardening_coefficient * BK);

      std::cout<<"lambda dot = "<<lambda_dot<<"\n";

      for (unsigned int i =0; i<3; ++i)
      {
        force_increment(i) = -(1/Q_F(i)) * (residual_force_vector(i) + lambda_dot * _material_stiffness[_qp](i) * dphidF(i));
        moment_increment(i) = -(1/Q_M(i)) * (residual_moment_vector(i) + lambda_dot * _material_flexure[_qp](i) * dphidM(i));
      }

      F += force_increment;
      M += moment_increment;
      lambda += lambda_dot;

      for(unsigned int i = 0; i<3; ++i)
      {
        _iso_hardening_variable_force[_qp](i) = _iso_hardening_variable_force_old[_qp](i) + _hardening_constant * _isotropic_hardening_coefficient * _material_stiffness[_qp](i) * std::abs(lambda * dphidF(i));
        _iso_hardening_variable_moment[_qp](i) = _iso_hardening_variable_moment_old[_qp](i) + _hardening_constant * _isotropic_hardening_coefficient * _material_flexure[_qp](i) * std::abs(lambda * dphidM(i));
        _kin_hardening_variable_force[_qp](i) = _kin_hardening_variable_force_old[_qp](i) + (1 - _hardening_constant) * _kinematic_hardening_coefficient * _material_stiffness[_qp](i) * lambda * dphidF(i);
        _kin_hardening_variable_moment[_qp](i) = _kin_hardening_variable_moment_old[_qp](i) + (1 - _hardening_constant) * _kinematic_hardening_coefficient * _material_flexure[_qp](i) * lambda * dphidM(i);
      }

      ++iteration;
      if(iteration > _max_its)
        throw MooseException("NonlinearBeam: Plasticity model did not converge");

      // std::cout<<"F = "<<F<<"\n";
      // std::cout<<"M = "<<M<<"\n\n";
      // std::cout<<"hard var = "<<_iso_hardening_variable_moment[_qp]<<"\n";

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
