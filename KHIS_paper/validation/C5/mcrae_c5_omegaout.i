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
  [column]
    type = FileMeshGenerator
    file = mcrae_base_refine.e
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[AuxKernels]
  [omega]
    type = MaterialRealAux
    property = viz
    variable = omega
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
    generate_output = 'stress_xx strain_xx plastic_strain_xx vonmises_stress effective_plastic_strain'
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
  [omega]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Functions]
  [disp]
    type = PiecewiseLinear
    x = '0  1   3  5  7     8 9 11  13  15  16      17  19  21  23  24    25  27  29  31  32    33  35  37  39  40    41  43  45  47'
    y = '0  2.11  -2.11 2.11 -2.11  0   6.5 -6.5  6.5 -6.5  0   11.7  -11.7 11.7  -11.7 0   16.8  -16.8 16.8  -16.8 0   22.1  -22.1 22.1  -22.1 0   27.3  -27.2 27.2  -4.3'
  []
  [load]
    type = ConstantFunction
    value = 150
  []
[]

[Materials]
  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'plasticity'
  []
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 200000
    poissons_ratio = 0
  [../]
  [./plasticity]
    type = SelectiveHardeningStressUpdate
    yield_stress = 315
    hardening_constant = 1200
    deterioration_constant = -200
    gamma = 1.142857
    output_properties = viz
    outputs = exodus
    use_substep = true
  [../]
[]

[BCs]
  [fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = '2'
    value = 0
  []
  [fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = '2'
    value = 0
  []
  [fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = '2'
    value = 0
  []
  [lateral]
    type = DirichletBC
    variable = disp_x
    boundary = '3'
    # boundary = '4 5'
    value = 0.0
  []
  [disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = '1'
    function = 'disp'
    # preset = false
  []
  [./Pressure]
    [top]
      boundary = '1'
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
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -ksp_gmres_restart -pc_hypre_boomeramg_strong_threshold -pc_hypre_boomeramg_interp_type -pc_hypre_boomeramg_coarsen_type -pc_hypre_boomeramg_agg_nl -pc_hypre_boomeramg_agg_num_paths -pc_hypre_boomeramg_truncfactor -pc_factor_shift_amount'
  petsc_options_value = 'hypre boomeramg 200 0.7 ext+i PMIS 4 2 0.4 1e-10'

  automatic_scaling = true
  l_max_its = 100
  l_tol = 1e-6
  nl_max_its = 15
  nl_rel_tol = 1e-4
  nl_abs_tol = 5e-4
  nl_forced_its = 1

  start_time = 0
  end_time = 47
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.01
    timestep_limiting_function = 'disp'
    max_function_change = 1
    optimal_iterations = 10
    iteration_window = 5
    force_step_every_function_point = true
    cutback_factor = 0.8
    growth_factor = 1.25
    linear_iteration_ratio = 1000
  []
[]

[Postprocessors]
  [disp_y]
    type = NodalMaxValue
    boundary = '1'
    variable = disp_y
  []
  [force_y]
    type = NodalSum
    variable = saved_y
    boundary = 2
  []
  # [force_z]
  #   type = NodalSum
  #   variable = saved_z
  #   boundary = 2
  # []
  # [dt]
  #   type = TimestepSize
  # []
[]

[Outputs]
  csv = true
  exodus = true
  perf_graph = true
[]
