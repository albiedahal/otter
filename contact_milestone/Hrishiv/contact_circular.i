
[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y'
[]


[Mesh]
  file = circular.e
  displacements = 'disp_x disp_y'
  patch_update_strategy = iteration
  allow_renumbering = false
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


[Functions]
  [./dispy]
    type = PiecewiseLinear
    x = '0.  1.0 2'
    y = '0.  -0.25 0'
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
    generate_output = 'stress_xx stress_xy stress_xz stress_yy stress_zz'
    save_in = 'saved_x saved_y'
  [../]
[]

[AuxKernels]
  [./nor_forc_y]
  type = PenetrationAux
  variable = nor_forc_y
  quantity = normal_force_y
  boundary = 10
  paired_boundary = 11
[../]
[./nor_forc_x]
  type = PenetrationAux
  variable = nor_forc_x
  quantity = normal_force_x
  boundary = 10
  paired_boundary = 11
[../]
[./tang_forc_x]
  type = PenetrationAux
  variable = tang_forc_x
  quantity = tangential_force_x
  boundary = 10
  paired_boundary = 11
[../]
[./tang_forc_y]
  type = PenetrationAux
  variable = tang_forc_y
  quantity = tangential_force_y
  boundary = 10
  paired_boundary = 11
[../]
[]

[BCs]
[./fixy_bot]
  type = DirichletBC
  variable = disp_y
  boundary = '1'
  value = 0.0
[../]
[./disp_y]
  type = FunctionDirichletBC
  variable = disp_y
  boundary = 101
  function = dispy
[../]
[]


[Contact]
  [./rightcontact]
    primary = 11
    secondary = 10
    model = coulomb
    formulation = penalty
    normalize_penalty = true
    friction_coefficient = 0.3
    penalty = 1e8
    tangential_tolerance = 0.005
  [../]
[]


[Dampers]
  [./contact_slip1]
    type = ContactSlipDamper
    secondary = 10
    primary = 11
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
  petsc_options_value = 'ilu      101'  # use this without petsc options
  nl_abs_tol = 1e-06
  nl_rel_tol = 1e-07
  l_max_its = 40
  start_time = 0.0
  dt = 0.025
  dtmin = 0.000025
  end_time = 2
[]


[Outputs]
  file_base = contactcircular_frictionreversal
  perf_graph = true
  # csv = true
  exodus = true
  print_linear_residuals = false
[]
