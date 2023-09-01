//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonlocalTruss.h"
#include "Function.h"
#include "MooseException.h"
#include "MathUtils.h"

registerMooseObject("TensorMechanicsApp", NonlocalTruss);

InputParameters
NonlocalTruss::validParams()
{
  InputParameters params = LinearElasticTruss::validParams();
  params.addClassDescription(
      "Computes the stress and strain for a truss element with plastic behavior defined by either "
      "linear hardening or a user-defined hardening function.");
  params.addRequiredParam<Real>("yield_stress",
                                "Yield stress after which plastic strain starts accumulating");
  params.addParam<Real>("hardening_constant", 0.0, "Hardening slope");
  params.addParam<FunctionName>("hardening_function",
                                "Engineering stress as a function of plastic strain");
  params.addRequiredParam<Real>("characteristic_length", "Characteristic length for the material");
  params.addParam<Real>("alpha",-16.0,"Regularization Parameter");
  params.addParam<Real>(
      "absolute_tolerance", 1e-10, "Absolute convergence tolerance for Newton iteration");
  params.addParam<Real>(
      "relative_tolerance", 1e-8, "Relative convergence tolerance for Newton iteration");
  return params;
}

NonlocalTruss::NonlocalTruss(const InputParameters & parameters)
  : LinearElasticTruss(parameters),
    _yield_stress(getParam<Real>("yield_stress")), // Read from input file
    _hardening_constant(getParam<Real>("hardening_constant")),
    _hardening_function(isParamValid("hardening_function") ? &getFunction("hardening_function")
                                                           : NULL),
    _chlen(getParam<Real>("characteristic_length")),
    _alpha(getParam<Real>("alpha")),
    _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    _total_stretch_old(getMaterialPropertyOld<Real>(_base_name + "total_stretch")),
    _plastic_strain(declareProperty<Real>(_base_name + "plastic_stretch")),
    _plastic_strain_old(getMaterialPropertyOld<Real>(_base_name + "plastic_stretch")),
    _plastic_strain_nlc(declareProperty<Real>(_base_name + "plastic_stretch_nlc")),
    _plastic_strain_nlc_old(getMaterialPropertyOld<Real>(_base_name + "plastic_stretch_nlc")),
    _stress_old(getMaterialPropertyOld<Real>(_base_name + "axial_stress")),
    _pos(),
    _wt(),
    _strial(),
    _ystress(),
    _et_lc(),
    _p_init(),
    // _p_old(getMaterialPropertyOld<std::vector<Real>>("p")),
    // _p_nlc(declareProperty<std::vector<Real>>("pnlc")),
    // _p_nlc_old(getMaterialPropertyOld<std::vector<Real>>("pnlc")),
    _hardening_variable(declareProperty<Real>(_base_name + "hardening_variable")),
    _hardening_variable_old(getMaterialPropertyOld<Real>(_base_name + "hardening_variable")),
    _max_its(1000)
{
  if (!parameters.isParamSetByUser("hardening_constant") && !isParamValid("hardening_function"))
    mooseError("NonlocalTruss: Either hardening_constant or hardening_function must be defined");

  if (parameters.isParamSetByUser("hardening_constant") && isParamValid("hardening_function"))
    mooseError("NonlocalTruss: Only the hardening_constant or only the hardening_function can be "
               "defined but not both");

  nelem = _mesh.nElem();

  _pos.resize(nelem);
  _wt.resize(nelem);
  _strial.resize(nelem);
  _ystress.resize(nelem);
  _et_lc.resize(nelem);
  _p_init.resize(nelem);

  for (unsigned int i = 0; i < nelem; ++i)
  {
    _pos[i] = &declareProperty<Real>("pos" + Moose::stringify(i));
    _wt[i] = &declareProperty<Real>("wt" + Moose::stringify(i));
    _strial[i] = &declareProperty<Real>("strial" + Moose::stringify(i));
    _ystress[i] = &declareProperty<Real>("ystress" + Moose::stringify(i));
    _et_lc[i] = &declareProperty<Real>("et_lc" + Moose::stringify(i));
    _p_init[i] = &declareProperty<Real>("p_init" + Moose::stringify(i));
  }
}

void
NonlocalTruss::initQpStatefulProperties()
{
  TrussMaterial::initQpStatefulProperties();

  // std::cout << "el ptr =" << _mesh.elemPtr(1) << "\n";
  // std::cout << "el =" << _current_elem << "\n";

  _plastic_strain[_qp] = 0.0;
  _plastic_strain_nlc[_qp] = 0.0;
  _hardening_variable[_qp] = 0.0;

  // nelem = _mesh.nElem();
  _phi_nlc.resize(nelem,0);
  _q_pos.resize(nelem,0);
  _w.resize(nelem,0);

  _p_nlc.resize(nelem);
  _p_lc.resize(nelem);
  _et_lc.resize(nelem);
  t_stress.resize(nelem,0);
  _lambda.resize(nelem);

  for (unsigned int i = 0; i < nelem; ++i)
  {
   (*_pos[i])[_qp] = 0.0;
   (*_wt[i])[_qp] = 0.0;
   (*_strial[i])[_qp] = 0.0;
   (*_ystress[i])[_qp] = 0.0;
   (*_et_lc[i])[_qp] = 0.0;
   (*_p_init[i])[_qp] = 0.0;
   // std::cout<<"\n itit pos "<<i<<" = "<<(*_pos[i])[_qp]<<"\n";
  }
}

void
NonlocalTruss::computeQpStrain()
{
  // std::cout << "\n************ Element id ="<<_current_elem -> id()<<"******************  \n";
  // std::cout << "MATERIAL COMPUTATION CALLED \n";

  NonlinearSystemBase & nonlinear_sys = _fe_problem.getNonlinearSystemBase();
  const NumericVector<Number> & sol = *nonlinear_sys.currentSolution();
  const NumericVector<Number> & sol_old = nonlinear_sys.solutionOld();

  // std::cout<<"\n current soln ="<<sol<<"\n";
  // std::cout<<"\n old soln ="<<sol_old<<"\n";
  // std::cout<<" total stretch = "<< _total_stretch[_qp] <<"\n";

  _total_stretch[_qp] = _current_length / _origin_length - 1.0;

  computeNonlocalVars();
}



void
NonlocalTruss::computeQpStress()
{
  // std::cout<<"stress of element = " << _axial_stress[_qp] << "\n \n";
}


void
NonlocalTruss::computeNonlocalVars()
{
  // std::cout << "\n  original size of vector from nlc = " << _q_pos.size() << "\n";

  Real idx = _current_elem -> id();
  _q_pos[idx] = _q_point[_qp](0);
  _w[idx] = _JxW[_qp] * _coord[_qp];

  (*_pos[idx])[_qp] = _q_point[_qp](0);
  (*_wt[idx])[_qp] = _JxW[_qp] * _coord[_qp];
  (*_ystress[idx])[_qp] = _yield_stress;

  // std::cout<<"\n q_pos = "<<_q_point[_qp];
  // std::cout<<"\n weight = "<<_w[idx]<<"\n";

  _wfn.resize(_q_pos.size(),_q_pos.size());
  _wfnt.resize(_q_pos.size(),_q_pos.size());
  _t1.resize(_q_pos.size(),_q_pos.size());
  _t2.resize(_q_pos.size());
  _waw.resize(_q_pos.size(),_q_pos.size());
  _aw.resize(_q_pos.size(),_q_pos.size());
  _wawaw.resize(_q_pos.size(),_q_pos.size());

  for (MooseIndex(_q_pos) i = 0; i < _q_pos.size(); i++)
  {
    // std::cout<<"\n qpos (old way) = "<<_q_pos[i]<<"\n";
    // std::cout<<"\n qpos (new way)= "<<(*_pos[i])[_qp]<<"\n";
    // std::cout<<"\n w = "<<(*_wt[i])[_qp]<<"\n";
    for (MooseIndex(_q_pos) j = 0; j < _q_pos.size(); j++)
    {
      _wfn(i,j) = (1/(_chlen * std::sqrt(2 * M_PI))) * std::exp(-std::pow(((*_pos[i])[_qp] - (*_pos[j])[_qp]),2) / (2 * std::pow(_chlen,2)));
      // std::cout << "A " << i << j << " = " << _wfn(i,j) << "\n";
    }
  }

  _wfn.get_transpose(_wfnt);

  for (MooseIndex(_q_pos) i = 0; i < _q_pos.size(); i++)
  {
    // std::cout << "q pos = " << _q_pos[i] << "\n";
    for (MooseIndex(_q_pos) j = 0; j < _q_pos.size(); j++)
    {
      // _waw(i,j) = _w[i] * _wfnt(i,j) * _w[i];
      // _aw(i,j) = _wfn(i,j) * _w[i];
      _waw(i,j) = (*_wt[i])[_qp] * _wfnt(i,j) * (*_wt[i])[_qp];
      _aw(i,j) = _wfn(i,j) * (*_wt[i])[_qp];
      // std::cout << "waw " << i << j << "= "<< _waw(i,j) << "\n";
    }
  }


  for (MooseIndex(_q_pos) i = 0; i < _q_pos.size(); i++)
  {
    for (MooseIndex(_q_pos) j = 0; j < _q_pos.size(); j++)
    {
      for(MooseIndex(_q_pos) k = 0; k < _q_pos.size(); k++)
      {
        _wawaw(i,j) += _waw(i,k) * _aw(k,j);
      }
      // std::cout << "wawaw " << i << j << "= "<< _wawaw(i,j) << "   ";
    }
    // std::cout<<"\n";
  }

  for (MooseIndex(_q_pos) i = 0; i < _q_pos.size(); i++)
  {
    for (MooseIndex(_q_pos) j = 0; j < _q_pos.size(); j++)
    {
      _t1(i,j) = _wawaw(i,j) + _waw(i,j) *  _youngs_modulus[_qp] / _hardening_constant;
      if(i == j)
      {
        // _t1(i,j) += _w[i] * _alpha;
        _t1(i,j) += (*_wt[i])[_qp] * _alpha;
      }
      // std::cout << "t1 " << i << j << "= "<< _t1(i,j) << "   ";
    }
    // std::cout<<"\n";
  }

  computeNonlocalStress(_t1, _waw, _lambda, _wt, _wfn);
  // computeNonlocalStress(_t1, _waw, _lambda, _w, _wfn);

  // std::cout<<"\n idx = " << idx << "\n plc = "<<_p_lc[idx]<<"\n";

  _plastic_strain[_qp] = _p_lc[idx];

  // std::cout << "\n***strored value of ep at _qp " << _qp << " = " << _plastic_strain[_qp] << "\n";
  // std::cout << "\n***strored value of et at _qp " << _qp << " = " << _total_stretch[_qp] << "\n";

  _elastic_stretch[_qp] = _total_stretch[_qp] - _plastic_strain[_qp];
  _axial_stress[_qp] = _youngs_modulus[_qp] * _elastic_stretch[_qp];

  // std::cout << "***strored value of sigma at _qp " << _qp << " = " << _axial_stress[_qp] << "\n";

}

std::vector<Real>
NonlocalTruss::computeNonlocalStress(DenseMatrix<Real> t1,DenseMatrix<Real> waw,DenseVector<Real> _lambda, std::vector<MaterialProperty<Real> *> _wt, DenseMatrix<Real> A)
// std::vector<Real>
// NonlocalTruss::computeNonlocalStress(DenseMatrix<Real> t1,DenseMatrix<Real> waw,DenseVector<Real> _lambda, std::vector<Real> _wt, DenseMatrix<Real> A)
{
  int its = 1;

  _plastic_strain[_qp] = _plastic_strain_old[_qp];
  _plastic_strain_nlc[_qp] = _plastic_strain_nlc_old[_qp];

  Real strain_increment = _total_stretch[_qp] - _total_stretch_old[_qp];
  Real idx = _current_elem -> id();
  // _strial[idx] = (_youngs_modulus[_qp] * (_total_stretch[_qp] - _plastic_strain[_qp]));
  (*_strial[idx])[_qp] = (_youngs_modulus[_qp] * (_total_stretch[_qp] - _plastic_strain[_qp]));
  (*_p_init[idx])[_qp] = _plastic_strain_old[_qp];
  (*_et_lc[idx])[_qp] = _total_stretch[_qp];

  for (MooseIndex(_phi_nlc) j = 0; j < _phi_nlc.size(); j++)
  {
    // std::cout << "Initial trial stress " << j << "= "<< _strial[j] << "\n";
    // std::cout << "Initial trial stress " << j << "= "<< (*_strial[j])[_qp] << "\n";
    // std::cout << "et " << j << "= "<< (*_et_lc[j])[_qp] << "\n";
  }


  while (true)
  {
    // std::cout << "\n Iteration counter = " << its << "\n";

    Real temp = 0;
    Real len = 0;
    if(its == 1)
    {
      for (MooseIndex(t_stress) i = 0; i < t_stress.size(); i++)
      {
        t_stress[i] = (*_strial[i])[_qp];
        // t_stress[i] = _strial[i];
        _p_lc[i] = (*_p_init[i])[_qp];

      }
    }
    else
    {
      for (MooseIndex(_phi_nlc) i = 0; i < _phi_nlc.size(); i++)
      {
        t_stress[i] = _youngs_modulus[_qp] * ((*_et_lc[i])[_qp] - _p_lc[i]);
      }
      // std::cout<<"\n";
    }

    // for (MooseIndex(_phi_nlc) i = 0; i < _phi_nlc.size(); i++)
    // {
    //   std::cout<<"et "<<its<<" = "<<(*_et_lc[i])[_qp]<<"   "<<"ep = "<<_p_lc[i]<<"   ";
    //   // std::cout << " t stress <<" << t_stress[i]<< "   ";
    // }
    // std::cout<<"\n";

    for (MooseIndex(_phi_nlc) i = 0; i < _phi_nlc.size(); i++)
    {
      _p_nlc[i] = 0;
      for(MooseIndex(_phi_nlc) j = 0; j < _phi_nlc.size(); j++)
      {
        // std::cout<<"ep local = "<<_p_lc[j]<<"\n";
        _p_nlc[i] += A(i,j) * _p_lc[j] * (*_wt[j])[_qp];
        // _p_nlc[i] += A(i,j) * _p_lc[j] * _w[j];
      }

      _phi_nlc[i] = std::abs(t_stress[i]) - (*_ystress[i])[_qp] - _hardening_constant * _p_nlc[i];

      // for (MooseIndex(_phi_nlc) i = 0; i < _phi_nlc.size(); i++)
      // {
      //   std::cout << " phi par " << " = " << _phi_nlc[i] << "   "<<   t_stress[i]<< "   "<<(*_ystress[i])[_qp]<< "   "<<_p_nlc[i]<<"\n";
      // }

      if(_phi_nlc[i] < 0)
      {
        _phi_nlc[i] = 0;
      }
      // temp += std::pow(_phi_nlc[i],2) * _w[i];
      temp += std::pow(_phi_nlc[i],2) * (*_wt[i])[_qp];
      len += _current_elem -> hmax();
    }

    // for (MooseIndex(_phi_nlc) i = 0; i < _phi_nlc.size(); i++)
    // {
    //   std::cout << " phi " << i << " = " << _phi_nlc[i] << "   ";
    // }
    // std::cout<<"\n";

    Real _res = std::sqrt(temp/len);

    // std::cout << "\n res = " << _res << "\n";

    if(_res < 1e-3 || its > 1000)
    {
      break;
    }


    for (MooseIndex(_phi_nlc) i = 0; i < _phi_nlc.size(); i++)
    {
      _t2(i) = 0;
      for (MooseIndex(_w) j = 0; j < _w.size(); j++)
      {
        _t2(i) += _waw(i,j) * _phi_nlc[j] / _hardening_constant;
        // std::cout<<_waw(i,j)<<"    "<<_t2(i)<<"   ";
      }
      // std::cout << "t2 " << i << "= "<< _t2(i) << "\n";
    }

    _t1.lu_solve(_t2,_lambda);

    for (MooseIndex(_phi_nlc) i = 0; i < _phi_nlc.size(); i++)
    {
      // std::cout << "inc. plastic strain = " << _lambda(i) << "\n";
      _p_lc[i] += _lambda(i) * MathUtils::sign(t_stress[i]);
    }

    its++;
  }

  // std::cout << "\n ****Return map convergence**** \n";

  for (MooseIndex(_lambda) j = 0; j < _lambda.size(); j++)
  {
    // std::cout << "final plastic strain = " << _p_lc[j] << "\n";
  }
  return _p_lc;
}
