# Steady-state analysis of a cantilever beam using 1D element
# The beam is made of Aluminum.
# Young's Modulus = 73.1 GPa
# Poisson's Ratio =  0.33
# Beam Dimensions = 1*0.1*0.1 m^3
# Load = 5000N at free end


[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 3000
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

[AuxVariables]
  [forcey]
    order = FIRST
    family = LAGRANGE
  [../]
  [momentz]
    order = FIRST
    family = LAGRANGE
  [../]
[../]

# [NodalKernels]
#   [force_y2]
#     type = UserForcingFunctionNodalKernel
#     function = '-1000'
#     variable = disp_y
#     boundary = 'right'
#   []
#   [force_x2]
#     type = UserForcingFunctionNodalKernel
#     function = '-1000'
#     variable = disp_z
#     boundary = 'right'
#   []
# []

[Functions]
  [load]
    type = PiecewiseLinear
    x = '0   1   2   3   4  5  6  7  8   9  10 11 12 13 14 15'
    y = '0   30  60  90  60  30  0 -30 -60 -90  -60 -30  0  30  60 90'
  []
[]

[Materials]
  [elasticity]
    type = ComputeElasticityBeam
    poissons_ratio = 0.3
    youngs_modulus = 210
  []
  [strain]
    type = ComputeIncrementalBeamStrain
    Iz = 84375000
    Iy = 337500000
    area = 45000
    rotations = 'rot_x rot_y rot_z'
    displacements = 'disp_x disp_y disp_z'
    y_orientation = '0 1 0'
    outputs = exodus
    # output_properties = 'mech_disp_strain_increment mech_rot_strain_increment'
  []
  [stress]
    type = NonlinearBeam
    block = 0
    yield_force = '8700000000 8700000000 8700000000'
    yield_moments = '843750 843750 843750'
    isotropic_hardening_slope = 0.2
    kinematic_hardening_slope = 0.2
    outputs = exodus
    output_properties = 'forces moments'
    hardening_constant = 1
    # absolute_tolerance = 1e-3
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
    save_in = forcey
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
    save_in = momentz
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  # petsc_options = '-snes_ksp_ew'
  # petsc_options_iname = '-pc_type'
  # petsc_options_value = 'lu'
  # line_search = 'none'
  dt = 0.25
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
    point = '3000 0 0'
    variable = disp_y
  []
  [disp_y2]
    type = PointValue
    point = '1500 0 0'
    variable = disp_y
  []
  # [rotation1]
  #   type = PointValue
  #   point = '1 0 0'
  #   variable = rot_x
  # []
  [rotation2]
    type = PointValue
    point = '1500 0 0'
    variable = rot_z
  []
  [rotation3]
    type = PointValue
    point = '3000 0 0'
    variable = rot_z
  []
  # [intstr1]
  #   type = PointValue
  #   point = '0 0 0'
  #   variable = mech_disp_strain_increment_x
  # []
  # [intstr2]
  #   type = PointValue
  #   point = '1 0 0'
  #   variable = mech_disp_strain_increment_y
  # []
  # [pstr]
  #   type = PointValue
  #   point = '3000 0 0'
  #   variable = translational_plastic_strain_y
  # []
  [moments_z]
    type = PointValue
    point = '0 0 0'
    variable = moments_z
  []
  # [moments_z2]
  #   type = PointValue
  #   point = '1500 0 0'
  #   variable = moments_z
  # []
  # [moments_z3]
  #   type = PointValue
  #   point = '1 0 0'
  #   variable = moments_z
  # []
  [forces_y1]
    type = PointValue
    point = '0 0 0'
    variable = forces_y
  []
  # [forces_y2]
  #   type = PointValue
  #   point = '0.75 0 0'
  #   variable = forces_y
  # []
  # [res_forcey1]
  #   type = PointValue
  #   point = '1 0 0'
  #   variable = forcey
  # []
  # [res_forcey2]
  #   type = PointValue
  #   point = '0.75 0 0'
  #   variable = forcey
  # []
  # [res_momentz1]
  #   type = PointValue
  #   point = '1 0 0'
  #   variable = momentz
  # []
  # [res_momentz2]
  #   type = PointValue
  #   point = '0 0 0'
  #   variable = momentz
  # []
[]

  [Outputs]
    csv = true
    exodus = true
    perf_graph = true
  []
