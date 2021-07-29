
[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y'
[]



[Mesh]

  displacements = 'disp_x disp_y'
  patch_update_strategy = iteration
  allow_renumbering = false
  file = three_unbonded.e
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
    generate_output = 'stress_xx  stress_yy stress_zz vonmises_stress strain_zz'
    save_in = 'saved_x saved_y'
    use_automatic_differentiation = false
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
    quantity = normal_force_magnitude
    boundary = '8'
    paired_boundary = '9'
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
    penalty = 8e6
    tangential_tolerance = 0.005
  [../]
  [./leftcontact]
    primary = 7
    secondary = 6
    model = coulomb
    formulation = penalty
    normalize_penalty = true
    friction_coefficient = 0.3
    penalty = 8e6
    tangential_tolerance = 0.005
  [../]
  [./pellet12]
    primary = 11
    secondary = 10
    model = coulomb
    formulation = penalty
    normalize_penalty = true
    friction_coefficient = 0.3
    penalty = 8e6
    tangential_tolerance = 0.005
  [../]
  [./pellet23]
    primary = 12
    secondary = 13
    model = coulomb
    formulation = penalty
    normalize_penalty = true
    friction_coefficient = 0.3
    penalty = 8e6
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
  [./contact_slip3]
    type = ContactSlipDamper
    secondary = 10
    primary = 11
  [../]
  [./contact_slip4]
    type = ContactSlipDamper
    secondary = 13
    primary = 12
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
  petsc_options_iname = '-pc_type -ksp_gmres_restart'  # use this without petsc options
  petsc_options_value = 'lu      101'  # use this without petsc options
  # petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  # petsc_options_value = 'lu    superlu_dist'
  # petsc_options = '-mat_superlu_dist_iterrefine -mat_superlu_dist_replacetinypivot'
  line_search = 'none'
  nl_abs_tol = 1e-06
  nl_rel_tol = 1e-07
  l_max_its = 40
  start_time = 0.0
  dt = 0.025
  dtmin = 0.000025
  end_time = 1.0
[]



[Outputs]
  file_base = contact_unbonded1e8_gp
  print_linear_residuals = false
  perf_graph = true
  # csv = true
  exodus = true
[]
