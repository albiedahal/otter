#
#A stiff beam is pushed onto another soft beam
#

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = false
[]

[Mesh]
  file = twobeamfine.e
  patch_update_strategy = iteration
  # patch_size = 40
[]


[AuxVariables]
  [./saved_x]
  [../]
  [./saved_y]
  [../]
  [./saved_z]
  [../]
  [normal_force]
    order = FIRST
    family = LAGRANGE
  []
  [tangential_force]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [normal]
    type = PenetrationAux
    variable = normal_force
    quantity = normal_force_y
    boundary = 100
    paired_boundary = 13
  []
[]


[Functions]
  [./load]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 1'
    scale_factor = -2.5
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    strain = FINITE
    save_in = 'saved_x saved_y saved_z'
  [../]
[]

[Contact]
  [./dummy_name]
    primary = 13
    secondary = 100
    formulation = kinematic
    penalty = 1e3
  [../]
[]

[BCs]
  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = 101
    value = 0.0
  [../]

  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 101
    value = 0.0
  [../]

  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = 101
    value = 0.0
  [../]

  [./fixx]
    type = DirichletBC
    variable = disp_x
    boundary = 10
    value = 0
  [../]

  [./fixy]
    type = DirichletBC
    variable = disp_y
    boundary = 10
    value = 0
  [../]

  [./fixz]
    type = DirichletBC
    variable = disp_z
    boundary = 10
    value = 0
  [../]


  [./disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 12
    function = load
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 210
    poissons_ratio = 0.3
  [../]
  [./stiffStuff2]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 20
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  [../]
[]  # Materials

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu    superlu_dist'

  # petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  # petsc_options_value = 'hypre    boomeramg      101'

  # petsc_options = '-snes_ksp_ew'
  # petsc_options_iname = '-pc_type '
  # petsc_options_value = 'lu       '

  line_search = 'none'

  l_max_its = 60
  nl_max_its = 50
  dt = 0.025
  dtmin = 0.0001
  end_time = 1
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6 # 6 if no friction
  l_tol = 1e-3
  automatic_scaling = true
[]

[Postprocessors]
  [disp_beam]
    type = NodalExtremeValue
    variable = disp_y
    boundary = 12
    value_type = min
  []
  [disp_base]
    type = NodalExtremeValue
    variable = disp_y
    boundary = 100
    value_type = min
  []
  [normal]
    type = NodalSum
    variable = normal_force
    boundary = 100
  []
  [tang]
    type = NodalSum
    variable = tangential_force
    boundary = 100
  []
[]

[Outputs]
  perf_graph = true
  exodus = true
  csv = true
[]