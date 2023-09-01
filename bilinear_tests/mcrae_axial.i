[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
  group_variables = 'disp_x disp_y disp_z'
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [web]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -4.3
    xmax = 4.3
    ymin = -112.3
    ymax = 112.3
    nx = 2
    ny = 3
  []
  [flange_bottom]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -127
    xmax = 127
    ymin = -126.5
    ymax = -112.3
    nx = 3
    ny = 2
  []
  [flange_top]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -127
    xmax = 127
    ymin = 112.3
    ymax = 126.5
    nx = 3
    ny = 2
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'flange_bottom web flange_top'
    stitch_boundaries_pairs = 'top bottom;
                               top bottom'
  []
  [column]
    type = MeshExtruderGenerator
    input = stitch
    extrusion_vector = '0 0 1100'
    num_layers = 25
  []
[]

[Variables]
  [disp_x]
    # family = LAGRANGE
    # order = SECOND
  []
  [disp_y]
    # family = LAGRANGE
    # order = SECOND
  []
  [disp_z]
    # family = LAGRANGE
    # order = SECOND
  []
[]


[Modules/TensorMechanics/Master]
  [./all]
    strain = FINITE
    incremental = true
    add_variables = true
    displacements = 'disp_x disp_y disp_z'
    volumetric_locking_correction = true
    save_in = 'saved_x saved_y saved_z'
    # rotations = 'rot_x rot_y rot_z'
    generate_output = 'stress_xx strain_xx plastic_strain_xx vonmises_stress effective_plastic_strain'
    # generate_output = 'stress_xx stress_xy stress_xz stress_yx stress_yy stress_yz stress_zx stress_zy stress_zz strain_xx strain_xy strain_xz strain_yx strain_yy strain_yz strain_zx strain_zy strain_zz plastic_strain_xx vonmises_stress effective_plastic_strain'
    extra_vector_tags = 'ref'
  [../]
[]

[AuxVariables]
  [saved_x]
  []
  [saved_y]
  []
  [saved_z]
  []
[]
#

[Functions]
  [disp]
    type = PiecewiseLinear
    x = '0  3   9    15     21     27     33    39    45'
    y = '0  6  -6  12    -12     18    -18    24    -24'
  []
  [load]
    type = ConstantFunction
    value = 210
  []
[]

[Materials]
  [stress]
    # type = ComputeFiniteStrainElasticStress
    type = ComputeMultipleInelasticStress
    inelastic_models = 'kinematic_plasticity'
  []
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 200000
    poissons_ratio = 0
  [../]
  [./kinematic_plasticity]
    # type = CombinedHardeningStressUpdatel
    type = IsotropicPlasticityStressUpdate
    yield_stress = 300
    hardening_constant = 5210
    deterioration_constant = -590
    peak_strength = 490
    # hardening_function = hf
  [../]
[]

[BCs]
  [fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = '4'
    value = 0
  []
  [fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = '4'
    value = 0
  []
  [fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = '4'
    value = 0
  []
  # [fixx2]
  #   type = DirichletBC
  #   variable = disp_x
  #   boundary = '5'
  #   value = 0
  # []
  # [fixz2]
  #   type = DirichletBC
  #   variable = disp_z
  #   boundary = '5'
  #   value = 0
  # []

  [disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '5'
    function = 'disp'
    # preset = false
  []
  [./Pressure]
    [top]
      boundary = '5'
      displacements = 'disp_x disp_y disp_z'
      function = 'load'
    []
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
  # solve_type = 'PJFNK'
  #
  # petsc_options = '-snes_ksp_ew'
  # petsc_options_iname = '-ksp_gmres_restart'
  # petsc_options_value = '101'
  # l_max_its = 50
  # nl_max_its = 25
  #
  #
  # line_search = 'none'

  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -ksp_gmres_restart -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_interp_type -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_truncfactor -pc_factor_shift_amount'
  petsc_options_value = 'hypre boomeramg 200 0.7 ext+i PMIS 4 2 0.4 1e-10'

  automatic_scaling = false
  l_max_its = 150
  l_tol = 1e-6
  nl_max_its = 50
  nl_rel_tol = 1e-4
  nl_abs_tol = 5e-4
  nl_forced_its = 1

  dt = 0.01
  start_time = 0
  # num_steps = 1
  end_time = 45
  # dtmin = 0.001
[]

[Postprocessors]
  [disp_y]
    type = NodalMaxValue
    boundary = '5'
    variable = disp_y
  []
  [stress_xx]
    type = ElementAverageValue
    # type = ElementalVariableValue
    # point = '0 300 150'
    # elementid = 10
    variable = stress_xx
  []
  [strain_xx]
    type = ElementAverageValue
    variable = strain_xx
  []
  [force_x]
    type = NodalSum
    variable = saved_y
    boundary = 4
  []
  [mises]
    type = ElementAverageValue
    # type = ElementalVariableValue
    # type = PointValue
    # point = '0 0 0'
    # elementid = 0
    variable = vonmises_stress
  []
[]

[Outputs]
  csv = true
  exodus = true
  perf_graph = true
[]
