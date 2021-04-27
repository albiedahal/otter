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
  [cube]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 100
    ny = 8
    nz = 4
    xmin = 0
    xmax = 3000
    ymin = 0
    ymax = 300
    zmin = 0
    zmax = 150
  []
  # [cnode]
  #   type = ExtraNodesetGenerator
  #   coord = '3000 300 150'
  #   # coord = '3000 150 75'
  #   input = cube
  #   new_boundary = center
  # []

  partitioner = parmetis # I like this partitioner a lot
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_xy stress_xz stress_yx stress_yy stress_yz stress_zx stress_zy stress_zz strain_xx strain_xy strain_xz strain_yx strain_yy strain_yz strain_zx strain_zy strain_zz plastic_strain_xx vonmises_stress effective_plastic_strain'
    volumetric_locking_correction = true # this is pretty much always helpful when using HEX8
  []
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
  [load]
    type = PiecewiseLinear
    x = '0 5'
    y = '0 130'
  []
  [hf]
    type = PiecewiseLinear
    x = '0  0.001 0.01'
    y = '0.25 0.4 0.3'
  []
[]

[Materials]
  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = kinematic_plasticity
    # tangent_operator = elastic
  []
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 210
    poissons_ratio = 0.3
  []
  [kinematic_plasticity]
    type = KinematicPlasticityStressUpdate
    # type = KinematicPlasticityStressUpdateOld
    # type = IsotropicPlasticityStressUpdate
    # type = KinematicPlasticityStressUpdateRefined
    # type = Bilinear
    yield_stress = 0.25
    # hardening_constant = 150
    deterioration_constant = -11.1111111
    peak_strength = 0.4
    # threshold_strain = 0.001
    hardening_function = hf
  []
[]

[BCs]
  [fixx1]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [fixy1]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0
  []
  [fixz1]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0
  []
  # [pressure]
  #   type = Pressure
  #   boundary = top
  #   variable = disp_y
  #   component = 1
  #   factor = 5000
  # []
  [disp_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = right
    function = load
    # preset = false
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = TRUE
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type'
    petsc_options_value = ' gmres     asm      lu           NONZERO'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  scheme = explicit-euler
  end_time = 5
  dt = 0.25
  l_max_its = 200
  timestep_tolerance = 1e-06
  line_search = none
  petsc_options = '-ksp_snes_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = ' 201                hypre    boomeramg      4'
[]

[Postprocessors]
  [disp_y]
    type = NodalMaxValue
    boundary = right
    variable = disp_y
  []
  [stress_xx]
    type = PointValue
    point = '0 0 0'
    variable = stress_xx

  []
  [stress_yy]
    type = PointValue
    point = '0 0 0'
    variable = stress_yy
  []
  [stress_zz]
    type = PointValue
    point = '0 0 0'
    variable = stress_zz
  []
  [stress_xy]
    type = PointValue
    point = '0 0.05 0.05'
    variable = stress_xy
  []
  [stress_xz]
    type = PointValue
    point = '0 0.05 0.05'
    variable = stress_xz
  []
  [stress_yz]
    type = PointValue
    point = '0 0.05 0.05'
    variable = stress_yz
  []
  [strain_xx]
    type = PointValue
    point = '0 0 0'
    variable = strain_xx
  []
  [strain_yy]
    type = PointValue
    point = '0 0 0'
    variable = strain_yy
  []
  [strain_zz]
    type = PointValue
    point = '0 0 0'
    variable = strain_zz
  []
  [strain_xy]
    type = PointValue
    point = '0 0 0'
    variable = strain_xy
  []
  [strain_xz]
    type = PointValue
    point = '0 0 0'
    variable = strain_xz
  []
  [strain_yz]
    type = PointValue
    point = '0 0 0'
    variable = strain_yz
  []
  [mises]
    type = ElementExtremeValue
    # point = '0 0 0'
    variable = vonmises_stress
  []
[]

[Outputs]
  csv = true
  exodus = true
  perf_graph = true
[]
