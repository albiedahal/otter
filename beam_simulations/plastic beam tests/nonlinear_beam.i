# Steady-state analysis of a cantilever beam using 1D element
# The beam is made of Aluminum.
# Young's Modulus = 73.1 GPa
# Poisson's Ratio =  0.33
# Beam Dimensions = 1*0.1*0.1 m^3
# Load = 5000N at free end


[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  xmin = 0
  xmax = 1
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [rot_x]
  []
  [rot_y]
  []
  [rot_z]
  []
[]


# [NodalKernels]
#   [force_y2]
#     type = UserForcingFunctionNodalKernel
#     function = '-1100*t'
#     variable = disp_y
#     boundary = 'right'
#   []
# []

[Functions]
  [load]
    type = PiecewiseLinear
    x = '0   1       2      3      4      5   6    7      8     9       10      11 12   13     14     15'
    y = '0 0.0004 0.0008 0.0012 0.0008 0.0004 0 -0.0004 -0.0008 -.0012 -0.0008 -0.0004 0 0.0004 0.0008 0.0012'
  []
[]

[Materials]
  [elasticity]
    type = ComputeElasticityBeam
    poissons_ratio = 0.3
    youngs_modulus = 7.310e10
  []
  [strain]
    type = ComputeIncrementalBeamStrain
    Iy = 8.33e-6
    Iz = 8.33e-6
    area = 0.01
    rotations = 'rot_x rot_y rot_z'
    displacements = 'disp_x disp_y disp_z'
    y_orientation = '0 1 0'
    # yield_moment = '1100'
    # hardening_constant = '0'
  []
  [stress]
    type = NonlinearBeam
    block = 0
    yield_force = '8700000000 8700000000 8700000000'
    yield_moments = '1100 1100 1100'
    outputs = exodus
    # isotropic_hardening_coefficient = 1
    # kinematic_hardening_coefficient = 1
    kinematic_hardening_slope = 0.1
    isotropic_hardening_slope = 0.1
    hardening_constant = 1
    output_properties = 'forces moments'
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
  [fixr1]
    type = DirichletBC
    variable = rot_x
    boundary = left
    value = 0
  []
  [fixr2]
    type = DirichletBC
    variable = rot_y
    boundary = left
    value = 0
  []
  [fixr3]
    type = DirichletBC
    variable = rot_z
    boundary = left
    value = 0
  []
  [load]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = right
    # function = '0.0005*t'
    function = 'load'
  [../]
[]

[Kernels]
  [solid_disp_x]
    type = StressDivergenceBeam
    variable = disp_x
    rotations = 'rot_x rot_y rot_z'
    displacements = 'disp_x disp_y disp_z'
    component = 0
  []
  [solid_disp_y]
    type = StressDivergenceBeam
    variable = disp_y
    rotations = 'rot_x rot_y rot_z'
    displacements = 'disp_x disp_y disp_z'
    component = 1
  []
  [solid_disp_z]
    type = StressDivergenceBeam
    variable = disp_z
    rotations = 'rot_x rot_y rot_z'
    displacements = 'disp_x disp_y disp_z'
    component = 2
  []
  [solid_rot_x]
    type = StressDivergenceBeam
    variable = rot_x
    rotations = 'rot_x rot_y rot_z'
    displacements = 'disp_x disp_y disp_z'
    component = 3
  []
  [solid_rot_y]
    type = StressDivergenceBeam
    variable = rot_y
    rotations = 'rot_x rot_y rot_z'
    displacements = 'disp_x disp_y disp_z'
    component = 4
  []
  [solid_rot_z]
    type = StressDivergenceBeam
    variable = rot_z
    rotations = 'rot_x rot_y rot_z'
    displacements = 'disp_x disp_y disp_z'
    component = 5
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
    # petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_atol -snes_rtol -snes_max_it -ksp_atol -ksp_rtol -sub_pc_factor_shift_type'
    # petsc_options_value = 'gmres asm lu 1E-8 1E-8 25 1E-8 1E-8 NONZERO'

  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  # line_search = 'bt'
  dt = 0.1
  start_time = 0
  end_time = 15
  nl_abs_tol = 1e-8
[]

[Postprocessors]
  # [disp_x]
  #   type = PointValue
  #   point = '1 0 0'
  #   variable = disp_x
  # []
  [disp_y]
    type = PointValue
    point = '1 0 0'
    variable = disp_y
  []
  [rotation1]
    type = NodalMaxValue
    boundary = right
    variable = rot_z
  []
  # [rotation2]
  #   type = PointValue
  #   point = '0.5 0 0'
  #   variable = rot_z
  # []
  [forces_y]
    type = PointValue
    point = '1 0 0'
    variable = forces_y
  []
  [moments_z]
    type = PointValue
    point = '0 0 0'
    variable = moments_z
  [../]
  [moments_z2]
    type = PointValue
    point = '0.5 0 0'
    variable = moments_z
  [../]
  [moments_z3]
    type = PointValue
    point = '1 0 0'
    variable = moments_z
  [../]
  # [plasticstrian1]
  #   type = PointValue
  #   point = '1 0 0'
  #   variable = translational_plastic_strain_y
  # []
  # [plasticstrian2]
  #   type = PointValue
  #   point = '1 0 0'
  #   variable = rotational_plastic_strain_z
  # []
[]

  [Outputs]
    csv = true
    exodus = true
    perf_graph = true
  []
