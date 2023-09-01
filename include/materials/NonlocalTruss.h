//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LinearElasticTruss.h"
#include "DenseMatrix.h"

#include "libmesh/mesh.h"

class NonlocalTruss : public LinearElasticTruss
{
public:
  static InputParameters validParams();

  NonlocalTruss(const InputParameters & parameters);

protected:
  virtual void computeQpStrain();
  virtual void computeQpStress();
  virtual void initQpStatefulProperties();

  virtual void computeNonlocalVars();
  virtual std::vector<Real> computeNonlocalStress(DenseMatrix<Real> t1,DenseMatrix<Real> waw,DenseVector<Real> _lambda, std::vector<MaterialProperty<Real> *> wt, DenseMatrix<Real> A);
  // virtual std::vector<Real> computeNonlocalStress(DenseMatrix<Real> t1,DenseMatrix<Real> waw,DenseVector<Real> _lambda, std::vector<Real> _w, DenseMatrix<Real> A);

  //  yield stress and hardening property input
  Real _yield_stress;
  const Real _hardening_constant;
  const Function * const _hardening_function;
  const Real _chlen;
  const Real _alpha;

  /// convergence tolerance
  Real _absolute_tolerance;
  Real _relative_tolerance;

  const MaterialProperty<Real> & _total_stretch_old;
  MaterialProperty<Real> & _plastic_strain;
  const MaterialProperty<Real> & _plastic_strain_old;
  MaterialProperty<Real> & _plastic_strain_nlc;
  const MaterialProperty<Real> & _plastic_strain_nlc_old;
  const MaterialProperty<Real> & _stress_old;

  std::vector<MaterialProperty<Real> *> _pos;
  std::vector<MaterialProperty<Real> *> _wt;
  std::vector<MaterialProperty<Real> *> _strial;
  std::vector<MaterialProperty<Real> *> _ystress;
  std::vector<MaterialProperty<Real> *> _et_lc;
  std::vector<MaterialProperty<Real> *> _p_init;
  // const MaterialProperty<std::vector<Real>> & _p_old;
  // MaterialProperty<std::vector<Real>> & _p_nlc;
  // const MaterialProperty<std::vector<Real>> & _p_nlc_old;

  MaterialProperty<Real> & _hardening_variable;
  const MaterialProperty<Real> & _hardening_variable_old;

  std::vector<Real> _q_pos;
  std::vector<Real> _w;
  // std::vector<Real> _strial;
  std::vector<Real> _phi_nlc;
  std::vector<Real> _p_lc;
  std::vector<Real> _p_nlc;
  std::vector<Real> t_stress;

  Real nelem;



  DenseMatrix<Real> _wfn;
  DenseMatrix<Real> _wfnt;
  DenseMatrix<Real> _t1;
  DenseVector<Real> _t2;
  DenseMatrix<Real> _waw;
  DenseMatrix<Real> _aw;
  DenseMatrix<Real> _wawaw;



  DenseVector<Real> _lambda;


  /// maximum no. of iterations
  const unsigned int _max_its;
};
