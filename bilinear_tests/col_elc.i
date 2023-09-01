[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 25.0
  zmin = 0.0
  zmax = 1.0
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[AuxVariables]
  [./vel_x]
  [../]
  [./accel_x]
  [../]
  [./vel_y]
  [../]
  [./accel_y]
  [../]
  [./vel_z]
  [../]
  [./accel_z]
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./mises]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./DynamicTensorMechanics]
    displacements = 'disp_x disp_y disp_z'
    zeta = 0.0
  [../]
  [./inertia_x]
    type = InertialForce
    variable = disp_x
    velocity = vel_x
    acceleration = accel_x
    beta = 0.25
    gamma = 0.5
    eta= 0.0
  [../]
  [./inertia_y]
    type = InertialForce
    variable = disp_y
    velocity = vel_y
    acceleration = accel_y
    beta = 0.25
    gamma = 0.5
    eta= 0.6283
  [../]
  [./inertia_z]
    type = InertialForce
    variable = disp_z
    velocity = vel_z
    acceleration = accel_z
    beta = 0.25
    gamma = 0.5
    eta = 0.0
  [../]
[]

[AuxKernels]
  [./accel_x]
    type = NewmarkAccelAux
    variable = accel_x
    displacement = disp_x
    velocity = vel_x
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./vel_x]
    type = NewmarkVelAux
    variable = vel_x
    acceleration = accel_x
    gamma = 0.5
    execute_on = timestep_end
  [../]
  [./accel_y]
    type = NewmarkAccelAux
    variable = accel_y
    displacement = disp_y
    velocity = vel_y
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./vel_y]
    type = NewmarkVelAux
    variable = vel_y
    acceleration = accel_y
    gamma = 0.5
    execute_on = timestep_end
  [../]
  [./accel_z]
    type = NewmarkAccelAux
    variable = accel_z
    displacement = disp_z
    velocity = vel_z
    beta = 0.25
    execute_on = timestep_end
  [../]
  [./vel_z]
    type = NewmarkVelAux
    variable = vel_z
    acceleration = accel_z
    gamma = 0.5
    execute_on = timestep_end
  [../]
  [./stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
  [../]
  [./strain_yy]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_yy
    index_i = 1
    index_j = 1
  [../]
  [vonmises]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = mises
    scalar_type = VonMisesStress
    execute_on = timestep_end
  [../]
[]

[Functions]
  [./elcentro]
    type = PiecewiseLinear
    data_file = 'elcentro.csv'
    format = columns
    scale_factor = 981
  [../]
[]

[BCs]
  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0.0
  [../]
  [./top_x]
    type = DirichletBC
    variable = disp_x
    boundary = top
    value = 0.0
  [../]
  [./top_z]
    type = DirichletBC
    variable = disp_z
    boundary = top
    value = 0.0
  [../]
  [./acceleration]
    type = PresetAcceleration
    boundary = top
    function = elcentro
    variable = disp_y
    beta = 0.25
    acceleration = accel_y
    velocity = vel_y
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 250.0
    poissons_ratio = 1.0e-9
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'kinematic_plasticity'
  []
  [./kinematic_plasticity]
    # type = KinematicPlasticityStressUpdate
    # type = IsotropicPlasticityStressUpdate
    # type = Bilinear
    type = Bilin1
    yield_stress = 7.5
    hardening_constant = 25
    deterioration_constant = -12.5
    peak_strength = 12
    # hardening_function = hf
    # use_substep = true
  [../]
  [./density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = 0.0308
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = 'none'
  l_tol = 1e-8
  nl_max_its = 15
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  # [./TimeStepper]
  #   type = PostprocessorDT
    dt = 0.02
    dtmin = 0.001
  #   postprocessor = mat1
  # [../]
  start_time = 0.0
  end_time = 10
  timestep_tolerance = 1e-6
[]

[Postprocessors]
  [./disp_y]
    type = PointValue
    point = '0 25 0'
    variable = disp_y
    # boundary = top
  [../]
  [./vel_y]
    type = PointValue
    point = '0 25 0'
    variable = vel_y
  [../]
  [./accel_y]
    type = PointValue
    point = '0 25 0'
    variable = accel_y
  [../]
  [./ug]
    type = FunctionValuePostprocessor
    function = elcentro
  [../]
  [./sig_yy]
    type = PointValue
    point = '0 25 0'
    variable = stress_yy
  [../]
  [./eps_yy]
    type = PointValue
    point = '0 25 0'
    variable = strain_yy
  [../]
  [./mises]
    type = ElementAverageValue
    variable = mises
  [../]
  [sig_avg]
    type = ElementAverageValue
    variable = stress_yy
  []
  [eps_avg]
    type = ElementAverageValue
    variable = strain_yy
  []
  [mat1]
    type = MaterialTimeStepPostprocessor
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  print_perf_log = true
[]
