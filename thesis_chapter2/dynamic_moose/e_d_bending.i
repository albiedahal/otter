[Mesh]
  type = FileMesh
  file = beam_hex20.e
[]

# [GlobalParams]
#   displacements = 'disp_x disp_y disp_z'
# []

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
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xx]
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
    zeta = 0.019
    # add_variables = true
    volumetric_locking_correction = true
  [../]
  [./inertia_x]
    type = InertialForce
    variable = disp_x
    velocity = vel_x
    acceleration = accel_x
    beta = 0.25
    gamma = 0.5
    eta= 0.0704
  [../]
  [./inertia_y]
    type = InertialForce
    variable = disp_y
    velocity = vel_y
    acceleration = accel_y
    beta = 0.25
    gamma = 0.5
    eta= 0.0704
  [../]
  [./inertia_z]
    type = InertialForce
    variable = disp_z
    velocity = vel_z
    acceleration = accel_z
    beta = 0.25
    gamma = 0.5
    eta = 0.0704
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
  [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    selected_qp = 0
  [../]
  [./strain_xx]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_xx
    index_i = 0
    index_j = 0
    selected_qp = 0
  [../]
  [vonmises]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = mises
    scalar_type = VonMisesStress
    execute_on = timestep_end
    selected_qp = 0
  [../]
[]

[NodalKernels]
  [force]
    type = UserForcingFunctionNodalKernel
    function = triangular
    boundary = 1000
    variable = disp_y
  []
[]

[Functions]
  # [./elcentro]
  #   type = PiecewiseLinear
  #   data_file = 'elcentro.csv'
  #   format = columns
  #   scale_factor = 981
  # [../]
  # [sine]
  #   type = ParsedFunction
  #   value = 'if(t<10.0, -1*sin(pi*t/0.6), 0*t)'
  # []
  [triangular]
    type = PiecewiseLinear
    x = '0  2 6  10  14  16  32  40'
    y = '0  -0.025 0.025 -0.025  0.025 0  0   0'
  []
[]

[BCs]
  [./bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = 10
    value = 0.0
  [../]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 10
    value = 0.0
  [../]
  [./bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = 10
    value = 0.0
  [../]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 11
    value = 0.0
  [../]
  [./left_z]
    type = DirichletBC
    variable = disp_z
    boundary = 11
    value = 0.0
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 210
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ComputeIncrementalSmallStrain
    displacements = 'disp_x disp_y disp_z'
  [../]
  [stress]
    type = ComputeFiniteStrainElasticStress
    # inelastic_models = 'kinematic_plasticity'
  []
  # [./kinematic_plasticity]
  #   type = KinematicPlasticityStressUpdate
  #   yield_stress = 2
  #   hardening_constant = 25
  # [../]
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
  solve_type = PJFNK
  nl_rel_tol = 1e-08
  nl_abs_tol = 1e-08
  nl_max_its = 25
  # line_search = default
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = ' 201                hypre    boomeramg      4'
  dt = 0.1
  start_time = 0
  end_time = 32
[]

[Postprocessors]
  [./disp_xs]
    type = NodalVariableValue
    nodeid = 611
    variable = disp_y
    # boundary = top
  [../]
  [./sig_x1]
    type = ElementalVariableValue
    elementid = 2
    variable = stress_xx
  [../]
  [./e_x1]
    type = ElementalVariableValue
    elementid = 2
    variable = strain_xx
  [../]
  [./mises1]
    type = ElementalVariableValue
    elementid = 2
    variable = mises
  [../]
  [./sig_x2]
    type = ElementalVariableValue
    elementid = 38
    variable = stress_xx
  [../]
  [./e_x2]
    type = ElementalVariableValue
    elementid = 38
    variable = strain_xx
  [../]
  [./mises2]
    type = ElementalVariableValue
    elementid = 38
    variable = mises
  [../]
  [./sig_x3]
    type = ElementalVariableValue
    elementid = 78
    variable = stress_xx
  [../]
  [./e_x3]
    type = ElementalVariableValue
    elementid = 78
    variable = strain_xx
  [../]
  [./mises3]
    type = ElementalVariableValue
    elementid = 78
    variable = mises
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  print_perf_log = true
[]
