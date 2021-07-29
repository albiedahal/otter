[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = false
[]

[Mesh]
  file = cladding.e
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
    quantity = normal_force_magnitude
    boundary = 16
    paired_boundary = 12
  []
  [tangential]
    type = PenetrationAux
    variable = tangential_force
    quantity = tangential_force_magnitude
    boundary = 16
    paired_boundary = 12
  []
[]

[Functions]
  [./load]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 1'
    scale_factor = -3
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    strain = FINITE
    save_in = 'saved_x saved_y saved_z'
    block = '1 2'
  [../]
[]

[Contact]
  [./dummy_name]
    primary = 12
    secondary = 16
    model = coulomb
    formulation = kinematic
    penalty = 1e3
    friction_coefficient = 0.4
  [../]
[]

[Dampers]
  [contact_slip]
    type = ContactSlipDamper
    primary = 12
    secondary = 16
  []
[]

[BCs]
  [./fixx]
    type = DirichletBC
    variable = disp_x
    boundary = '10 11 14 15'
    value = 0
  [../]

  [./fixy]
    type = DirichletBC
    variable = disp_y
    boundary = '10 11 14 15'
    value = 0
  [../]

  [./fixz]
    type = DirichletBC
    variable = disp_z
    boundary = '10 11 14 15'
    value = 0
  [../]


  [./disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 100
    function = load
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 91000
    poissons_ratio = 0.33
  [../]
  [./stiffStuff2]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 91000
    poissons_ratio = 0.33
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

  # petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  # petsc_options_value = 'lu    superlu_dist'

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre    boomeramg      101'

  line_search = 'none'

  l_max_its = 60
  nl_max_its = 50
  dt = 0.025
  dtmin = 0.00001
  end_time = 1
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-6 # 6 if no friction
  l_tol = 1e-3
  automatic_scaling = true
[]

[Postprocessors]
  [disp]
    type = NodalMaxValue
    variable = disp_y
    boundary = 100
  []
  [resid_y]
    type = NodalSum
    variable = saved_y
    boundary = 100
  []
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
