
[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y'
[]



[Mesh]
  file = three_center_coarse.e
  displacements = 'disp_x disp_y'
  patch_update_strategy = iteration
  allow_renumbering = false
[]

[Functions]
  [./dispx]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 -0.2'
  [../]
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]

  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]

[]


[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    strain = FINITE
    block = '1 2'
    generate_output = 'stress_xx  stress_yy stress_zz vonmises_stress'
    save_in = 'saved_x saved_y'
    planar_formulation = PLANE_STRAIN
  [../]
[]

[AuxVariables]
  [./saved_x]
  [../]
  [./saved_y]
  [../]
  [./nor_forc_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./nor_forc_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./tang_forc_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./tang_forc_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./nor_forc_y]
    type = PenetrationAux
    variable = nor_forc_y
    quantity = normal_force_y
    boundary = 8
    paired_boundary = 9
  [../]
  [./nor_forc_x]
    type = PenetrationAux
    variable = nor_forc_x
    quantity = normal_force_x
    boundary = 8
    paired_boundary = 9
  [../]
  [./tang_forc_x]
    type = PenetrationAux
    variable = tang_forc_x
    quantity = tangential_force_x
    boundary = 8
    paired_boundary = 9
  [../]
  [./tang_forc_y]
    type = PenetrationAux
    variable = tang_forc_y
    quantity = tangential_force_y
    boundary = 8
    paired_boundary = 9
  [../]
[]

[BCs]

[./fixx_bot]
  type = DirichletBC
  variable = disp_x
  boundary = '1 2 3 4 5'
  value = 0.0
[../]

[./fixy_bot]
  type = DirichletBC
  variable = disp_y
  boundary = '1 2 3 4 5'
  value = 0.0
[../]

[./disp_y]
  type = FunctionDirichletBC
  variable = disp_x
  boundary = 50
  function = dispx
[../]

[]


[Contact]
  [./rightcontact]
    primary = 9
    secondary = 8
    model = coulomb
    formulation = penalty
    normalize_penalty = true
    friction_coefficient = 0.3
    penalty = 1e6
    tangential_tolerance = 0.005
  [../]
  [./leftcontact]
    primary = 7
    secondary = 6
    model = coulomb
    formulation = penalty
    normalize_penalty = true
    friction_coefficient = 0.3
    penalty = 1e6
    tangential_tolerance = 0.005
  [../]
[]


[Dampers]
  [./contact_slip1]
    type = ContactSlipDamper
    secondary = 8
    primary = 9
  [../]
  [./contact_slip2]
    type = ContactSlipDamper
    secondary = 7
    primary = 6
  [../]
[]

[Materials]
  [./pellet]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 201000
    poissons_ratio = 0.32
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  [../]

  [./cladding]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 91000
    poissons_ratio = 0.33
  [../]


[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  # petsc_options = '-snes_ksp_ew'
  #
  # petsc_options_iname = '-pc_type -snes_linesearch_type -pc_factor_shift_type -pc_factor_shift_amount'
  # petsc_options_value = 'lu       basic                 NONZERO               1e-15'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu    superlu_dist'
  petsc_options = '-mat_superlu_dist_iterrefine -mat_superlu_dist_replacetinypivot'
  line_search = 'none'
  # automatic_scaling = true
  nl_abs_tol = 1e-06
  nl_rel_tol = 1e-07
  l_max_its = 40
  start_time = 0.0
  dt = 0.025
  end_time = 1.0
[]


[Outputs]
    file_base = mid_samemesh_friction_coarse_ps
  print_linear_residuals = false
  perf_graph = true
  # csv = true
  exodus = true
[]
