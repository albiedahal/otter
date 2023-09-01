
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  # type = FileMesh
  # file = f_dbell.e

  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 4
  nz = 4
  xmax = 10
  elem_type = HEX8
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = SMALL
    incremental = true
    add_variables = true
    # volumetric_locking_correction = true
    generate_output = 'stress_xx stress_xy stress_xz stress_yx stress_yy stress_yz stress_zx stress_zy stress_zz strain_xx strain_xy strain_xz strain_yx strain_yy strain_yz strain_zx strain_zy strain_zz plastic_strain_xx vonmises_stress effective_plastic_strain'
  []
[]

[BCs]
  [symmy]
    type = DirichletBC
    variable = disp_y
    # boundary = 100
    boundary = bottom
    value = 0
  []
  [symmx]
    type = DirichletBC
    variable = disp_x
    # boundary = 100
    boundary = left
    value = 0
  []
  [symmz]
    type = DirichletBC
    variable = disp_z
    # boundary = 100
    boundary = back
    value = 0
  []
  # [constry]
  #   type = DirichletBC
  #   variable = disp_y
  #   boundary = right
  #   value = 0
  # []
  [axial_load]
    type = FunctionDirichletBC
    variable = disp_x
    # boundary = 101
    boundary = right
    function = load1
  []
  # [axial_load1]
  #   type = FunctionDirichletBC
  #   variable = disp_x
  #   boundary = 101
  #   # boundary = right
  #   function = load1
  # []
[]

[Functions]
  [./load]
    type = PiecewiseLinear
    x = '0   3          9        15       21       24     30        36'
    y = '0   0.0012    -0.0012   0.0014    -0.0010   0.0012 -0.0012  0.0014'
    # x = '0   3          9        15       21       24     30        36'
    # y = '0   0.0012    -0.0012   0.0014    -0.0014   0.0013 -0.0013  0.0015'
    # x = '0   3          9        15       21       24     30        36'
    # y = '0   0.001    -0.0005   0.0011    -0.0006   0.00115 -0.001  0.0013'


  [../]
  [./load1]
    type = PiecewiseLinear
    x = '0   3          9        15       21       24     30        36'
    y = '0   0.0005    -0.0005   0.0010    -0.0010   0.0012 -0.0012  0.0014'
    # x = '0   3          9        15       21       24     30        36'
    # y = '0   0.0012    -0.0012   0.0014    -0.0014   0.0013 -0.0013  0.0015'
    # x = '0   3          9        15       21       24     30        36'
    # y = '0   0.001    -0.0005   0.0011    -0.0006   0.00115 -0.001  0.0013'
    # scale_factor = -1

  [../]
  [hf]
    type = PiecewiseLinear
    x = '0 0.00004 0.0008'
    y = '200. 264. 200.'
  [../]
[]

[Materials]
  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'kinematic_plasticity'
  []
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 8e6
    poissons_ratio = 0
  [../]
  [./kinematic_plasticity]
    # type = IsotropicPlasticityStressUpdate
    # type = KinematicPlasticityStressUpdate
    type = CombinedHardeningStressUpdatel
    yield_stress = 200
    # hardening_constant = 0.3e6
    hardening_constant = 2e6
    # deterioration_constant = -1e6
    deterioration_constant = -0.5e6
    # peak_strength = 215.0
    peak_strength = 264.0

    # threshold_strain = 0.00004
    # hardening_function = hf
    # constant_on = ELEMENT
    # max_inelastic_increment = 0.001
  [../]
[]

[Postprocessors]
  [stress_xx]
    type = ElementAverageValue
    # type = PointValue
    # point = '0 0.5 0'
    variable = stress_xx
  []
  [strain_xx]
    type = ElementAverageValue
    # type = PointValue
    # point = '0 0.25 0'
    variable = strain_xx
  []
  [mises]
    type = PointValue
    point = '0 0.25 0'
    variable = vonmises_stress
  []
  [pe_eff]
    type = PointValue
    point = '0 0.25 0'
    variable = effective_plastic_strain
  []
  [mat1]
    type = MaterialTimeStepPostprocessor
  [../]
  [load]
    type = FunctionValuePostprocessor
    function = load
  []
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

  l_max_its  = 50
  l_tol      = 1e-2
  nl_max_its = 20
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-8

  # [./TimeStepper]
  #   type = PostprocessorDT
    dt = 0.01
    # dtmin = 0.001
  #   postprocessor = mat1
  # [../]
  petsc_options = '-ksp_snes_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = ' 201                hypre    boomeramg      4'
  start_time = 0
  end_time = 36
  # num_steps = 1362
[]

# [Executioner]
#   num_steps = 500
#   dt = 0.1
#   type = Transient
#
#   nl_rel_tol = 1e-6
#   nl_max_its = 10
#   l_tol = 1e-4
#   l_max_its = 50
#
#   solve_type = Newton
#   petsc_options_iname = '-pc_type'
#   petsc_options_value = 'lu'
# #  petsc_options_iname = '-pc_type -pc_hypre_type'
# #  petsc_options_value = 'hypre boomerang'
# #   automatic_scaling = true
# []

[Outputs]
  csv=true
  exodus = true
  console = true
[]
