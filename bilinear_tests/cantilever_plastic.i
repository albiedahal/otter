# Steady-state analysis of a cantilever beam using 3D element
# The beam is made of Aluminum.
# Young's Modulus = 73.1 GPa
# Poisson's Ratio =  0.33
# Beam Dimensions = 1*0.1*0.1 m^3
# Load = 5000N at free end


[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 20
  ny = 2
  nz = 2
  xmin = 0
  xmax = 200
  ymin = 0
  ymax = 20
  zmin = 0
  zmax = 20
  # elem_type = HEX8
  # elem_type = HEX27
[]

# [MeshModifiers]
#   [loadpoint]
#     type = AddExtraNodeset
#     new_boundary = 'center'
#     coord = '3000 150 75'
#   []
# []

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
    strain = SMALL
    incremental = true
    add_variables = true
    displacements = 'disp_x disp_y disp_z'
    volumetric_locking_correction = true
    save_in = 'saved_x saved_y saved_z'
    # rotations = 'rot_x rot_y rot_z'
    generate_output = 'stress_xx strain_xx plastic_strain_xx vonmises_stress effective_plastic_strain'
    # generate_output = 'stress_xx stress_xy stress_xz stress_yx stress_yy stress_yz stress_zx stress_zy stress_zz strain_xx strain_xy strain_xz strain_yx strain_yy strain_yz strain_zx strain_zy strain_zz plastic_strain_xx vonmises_stress effective_plastic_strain'
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
  [load]
    type = PiecewiseLinear
    x = '0  3   9    15     21     27     33    39    42'
    y = '0  22  -22  24    -24     25    -25    28    0'
  []
  [hf]
    type = PiecewiseLinear
    x = '0 0.001 0.01'
    y = '0.25 0.4 0.3'
  [../]
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
    type = CombinedHardeningStressUpdatel
    # type = KinematicPlasticityStressUpdate
    # type = IsotropicPlasticityStressUpdate
    yield_stress = 420
    hardening_constant = 1600
    deterioration_constant = -570
    peak_strength = 580
    # hardening_function = hf
  [../]
[]

[BCs]
  [fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = 'left'
    value = 0
  []
  [fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  []
  [fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = 'back'
    value = 0
  []
  # [fixy2]
  #   type = DirichletBC
  #   variable = disp_y
  #   boundary = 'right'
  #   value = 0
  # []
  # [fixz2]
  #   type = DirichletBC
  #   variable = disp_z
  #   boundary = 'right'
  #   value = 0
  # []

  # [pressure]
  #   type = Pressure
  #   boundary = top
  #   variable = disp_y
  #   component = 1
  #   factor = 5000
  # []
  [disp_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'right'
    function = 'load'
    # preset = false
  []
[]

[Preconditioning]
  [./SMP]
    type = 'SMP'
    full = TRUE
    # petsc_options = '-snes_ksp_ew'
    # petsc_options_iname = '-pc_type -sub_pc_type -pc_asm_overlap -ksp_gmres_restart'
    # petsc_options_value = 'asm lu 1 101'


#petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_atol -snes_rtol -snes_max_it -ksp_atol -ksp_rtol -sub_pc_factor_shift_type'
#  petsc_options_value = 'gmres asm lu 1E-8 1E-8 25 1E-8 1E-8 NONZERO'
  [../]
[]


# [Executioner]
#   type = Transient
#   solve_type = PJFNK
#   dt = 0.25
#   end_time = 15
#   nl_max_its = 50
# []

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  dt = 0.01
  start_time = 0
  # num_steps = 1
  end_time = 42
  # dtmin = 0.001
[]

[Postprocessors]
  [disp_x]
    type = NodalMaxValue
    boundary = 'right'
    variable = disp_x
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
  # [stress_yy]
  #   type = ElementAverageValue
  #   # type = ElementalVariableValue
  #   # point = '0 300 150'
  #   # elementid = 10
  #   variable = stress_yy
  # []
  # [strain_yy]
  #   type = ElementAverageValue
  #   variable = strain_yy
  # []
  # [stress_zz]
  #   type = ElementAverageValue
  #   # type = ElementalVariableValue
  #   # point = '0 300 150'
  #   # elementid = 10
  #   variable = stress_zz
  # []
  # [strain_zz]
  #   type = ElementAverageValue
  #   variable = strain_zz
  # []


  [force_x]
    type = NodalSum
    variable = saved_x
    boundary = right
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
