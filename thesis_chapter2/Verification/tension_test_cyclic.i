[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [cube]
  type = GeneratedMeshGenerator
  dim = 3
  nx = 5
  ny = 1
  nz = 1
  xmin = 0
  xmax = 5
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
  # elem_type = HEX20
  []
  # [cnode]
  #   type = ExtraNodesetGenerator
  #   coord = '3000 300 150'
  #   # coord = '3000 150 75'
  #   input = cube
  #   new_boundary = center
  # []
[]

# [Variables]
#   [disp_x]
#   []
#   [disp_y]
#   []
#   [disp_z]
#   []
# []

[AuxVariables]
  [stress_x]
    # order = CONSTANT
    # family = MONOMIAL
  []
  [strain_x]
    # order = CONSTANT
    # family = MONOMIAL
  []
  [s_vm]
  []
  [peeff]
  []
[]

[AuxKernels]
  [stress_x]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_x
    index_i = 0
    index_j = 0
    selected_qp = 0
  []
  [strain_x]
    type = RankTwoAux
    rank_two_tensor = total_strain
    variable = strain_x
    index_i = 0
    index_j = 0
    selected_qp = 0
  []
  [s_vm]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = s_vm
    scalar_type = VonMisesStress
    selected_qp = 0
  []
  [peeff]
    type = RankTwoScalarAux
    rank_two_tensor = plastic_strain
    variable = peeff
    scalar_type = EffectiveStrain
    selected_qp = 0
  []
[]

[Modules/TensorMechanics/Master]
  [./all]
    strain = SMALL
    incremental = true
    add_variables = true
    displacements = 'disp_x disp_y disp_z'
    # rotations = 'rot_x rot_y rot_z'
    generate_output = 'stress_xx strain_xx vonmises_stress effective_plastic_strain'
  [../]
[]


# [NodalKernels]
#   [force_y2]
#     type = UserForcingFunctionNodalKernel
#     function = load
#     variable = disp_y
#     boundary = center
#   []
# []

[Functions]
  [load1]
    type = PiecewiseLinear
    x = '0 1 3 5'
    y = '0 0.05 -0.05 0.01'
  []
  [load2]
    type = PiecewiseLinear
    x = '0 1 3 5'
    y = '0 -0.05 0.05 -0.01'
  []
[]

[Materials]
  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'kinematic_plasticity'
    # tangent_operator = 'elastic'
  []
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 210
    poissons_ratio = 0.3
  [../]
  [./kinematic_plasticity]
    type = KinematicPlasticityStressUpdate
    yield_stress = 0.25
    hardening_constant = 21
  [../]
[]

[BCs]
  # [fixx1]
  #   type = DirichletBC
  #   variable = disp_x
  #   boundary = 'left'
  #   value = 0
  # []
  # [fixy1]
  #   type = DirichletBC
  #   variable = disp_y
  #   boundary = 'top'
  #   value = 0
  # []
  # [fixz1]
  #   type = DirichletBC
  #   variable = disp_z
  #   boundary = 'back'
  #   value = 0
  # []
  [disp_x1]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'right'
    function = 'load1'
  []
  [disp_x2]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'left'
    function = 'load2'
  []
[]

[Preconditioning]
  [./SMP]
    type = 'SMP'
    full = TRUE
  [../]
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  nl_rel_tol = 1e-08
  nl_abs_tol = 1e-08
  nl_max_its = 25
  # line_search = default
  # petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  # petsc_options_value = ' 201                hypre    boomeramg      4'
  dt = 0.005
  start_time = 0
  end_time = 5
[]

[Postprocessors]
  [disp_x]
    type = NodalMaxValue
    boundary = 'right'
    variable = disp_x
  []
  [stress_xx]
    type = ElementalVariableValue
    variable = stress_xx
    elementid = 0
  []
  [strain_xx]
    type = ElementalVariableValue
    variable = strain_xx
    elementid = 0
  []
  [mises]
    type = ElementalVariableValue
    elementid = 0
    variable = vonmises_stress
  []
  [pe_eff]
    type = ElementalVariableValue
    elementid = 0
    variable = effective_plastic_strain
  []
  [stress_x]
    type = ElementalVariableValue
    variable = stress_x
    elementid = 0
  []
  [strain_x]
    type = ElementalVariableValue
    variable = strain_x
    elementid = 0
  []
  [s_vm]
    type = ElementalVariableValue
    elementid = 0
    variable = s_vm
  []
  [peeff]
    type = ElementalVariableValue
    elementid = 0
    variable = peeff
  []
[]

[Outputs]
  csv = true
  exodus = true
  perf_graph = true
[]
