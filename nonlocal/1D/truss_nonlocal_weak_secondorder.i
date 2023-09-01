[Mesh]
  [truss]
    type = GeneratedMeshGenerator
    dim = 1
    xmax = 100
    nx = 31
  []
  [center_element]
    type = ElementSubdomainIDGenerator
    input = truss
    subdomain_ids = 1
    element_ids = 15
    # type = ParsedSubdomainMeshGenerator
    # input = truss
    # combinatorial_geometry = 'x = 50'
    # block_id = 1
  []
[]

[GlobalParams]
  displacements = 'disp_x'
[]

[Variables]
  [./disp_x]
    # order = FIRST
    # family = LAGRANGE
  [../]
[]

[AuxVariables]
 [./axial_stress]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e_over_l]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./area]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./react_x]
    # order = FIRST
    # family = LAGRANGE
  [../]
  [total_stretch]
    order = CONSTANT
    family = MONOMIAL
  []
  [elastic_stretch]
    order = CONSTANT
    family = MONOMIAL
  []
  [plastic_stretch]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Functions]
  # [load]
  #   type = PiecewiseLinear
  #   x = '0 1'
  #   y = '0 0.011'
  # []
  [load]
    type = ParsedFunction
    value = 'if(t<=1.0,0.009,0.009 + (t-1)/4000)'
  []
[]

[BCs]
  [./fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./load]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = 'load'
    preset = 'false'
  [../]
[]

[AuxKernels]
  [./axial_stress]
    type = MaterialRealAux
    property = axial_stress
    variable = axial_stress
    block = '0 1'
  [../]
  [./e_over_l]
    type = MaterialRealAux
    property = e_over_l
    variable = e_over_l
    block = '0 1'
  [../]
  [./area]
    type = ConstantAux
    variable = area
    value = 1.0
    execute_on = 'initial timestep_begin'
    block = '0 1'
  [../]
  [total_stretch]
    type = MaterialRealAux
    property = total_stretch
    variable = total_stretch
    block = '0 1'
  []
  [elastic_stretch]
    type = MaterialRealAux
    property = elastic_stretch
    variable = elastic_stretch
    block = '0 1'
  []
  [plastic_stretch]
    type = MaterialRealAux
    property = plastic_stretch
    variable = plastic_stretch
    block = '0 1'
  []
[]

[Postprocessors]
  [./s_xx]
    type = PointValue
    point = '0 0 0'
    variable = axial_stress
  [../]
  [./e_xx]
    type = PointValue
    point = '0 0 0'
    variable = total_stretch
  [../]
  [./ee_xx]
    type = PointValue
    point = '0 0 0'
    variable = elastic_stretch
  [../]
  [./ep_xx]
    type = PointValue
    point = '0 0 0'
    variable = plastic_stretch
  [../]
  [reaction]
    type = PointValue
    point = '0 0 0'
    variable = react_x
  []
  # check for the response at the other end, to see if the distribution is same throughout the truss
  [./s_xx1]
    type = PointValue
    point = '100 0 0'
    variable = axial_stress
  [../]
  [./e_xx1]
    type = PointValue
    point = '100 0 0'
    variable = total_stretch
  [../]
  [./ee_xx1]
    type = PointValue
    point = '100 0 0'
    variable = elastic_stretch
  [../]
  [./ep_xx1]
    type = PointValue
    point = '100 0 0'
    variable = plastic_stretch
  [../]
  [reaction1]
    type = PointValue
    point = '100 0 0'
    variable = react_x
  []

  #weak element
  [./s_xx2]
    type = PointValue
    point = '50 0 0'
    variable = axial_stress
  [../]
  [./e_xx2]
    type = PointValue
    point = '50 0 0'
    variable = total_stretch
  [../]
  [./ee_xx2]
    type = PointValue
    point = '50 0 0'
    variable = elastic_stretch
  [../]
  [./ep_xx2]
    type = PointValue
    point = '50 0 0'
    variable = plastic_stretch
  [../]
  [reaction2]
    type = PointValue
    point = '50 0 0'
    variable = react_x
  []
  [disp]
    type = PointValue
    point = '100 0 0'
    variable = disp_x
  []
  # [./final_residual]
  #   type = Residual
  #   residual_type = final
  # [../]
  # [./initial_residual_before]
  #   type = Residual
  #   residual_type = initial_before_preset
  # [../]
  # [./initial_residual_after]
  #   type = Residual
  #   residual_type = initial_after_preset
  # [../]
[]

# [Preconditioning]
#   [./smp]
#     type = SMP
#     full = true
#     petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_atol -snes_rtol -snes_max_it -ksp_atol -ksp_rtol -sub_pc_factor_shift_type'
#     petsc_options_value = 'gmres asm lu 1E-8 1E-8 25 1E-8 1E-8 NONZERO'
#   [../]
# []

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  # l_max_its = 10000
  nl_max_its = 1000
  # l_max_its = 20
  dt = 1
  end_time = 25
  abort_on_solve_fail = true
  # petsc_options = -info
  line_search = none
  # line_search_package = moose
  # nl_div_tol = 1e+100
  # nl_abs_div_tol = 1e+1000
  # num_steps = 10
  # petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_atol -snes_rtol -snes_max_it -ksp_atol -ksp_rtol -sub_pc_factor_shift_type'
  # petsc_options_value = 'gmres asm lu 1E-8 1E-8 25 1E-8 1E-8 NONZERO'
  # petsc_options = -snes_monitor
  # petsc_options_iname = '-snes_type'
  # petsc_options_value = 'ms'
  # [Quadrature]
  #   type = GAUSS
  #   order = FIRST
  # []
[]

# [Executioner]
#   type = Transient
#   solve_type = 'PJFNK'
#   petsc_options = '-snes_ksp_ew'
#   petsc_options_iname = '-pc_type'
#   petsc_options_value = 'lu'
#   nl_abs_tol = 1e-8
#   l_max_its = 20
#   dt = 3
#   end_time = 3
#   # num_steps = 10
#   [Quadrature]
#     type = GAUSS
#     order = FIRST
#   []
# []


[Kernels]
  [./solid]
    type = StressDivergenceTensorsTruss
    component = 0
    variable = disp_x
    area = area
    save_in = react_x
    block = '0 1'
  [../]
[]

[Materials]
  [./truss1]
    # type = LinearElasticTruss
    # type = PlasticTruss
    type = NonlocalTruss
    youngs_modulus = 20000
    yield_stress = 2
    hardening_constant = -2000
    characteristic_length = 5
    alpha = -16
    block = '0'
    # outputs = exodus
  [../]
  [./truss2]
    type = NonlocalTruss
    youngs_modulus = 20000
    yield_stress = 1.8
    hardening_constant = -2000
    characteristic_length = 5
    alpha = -16
    block = '1'
    # outputs = exodus
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  print_perf_log = true
[]
