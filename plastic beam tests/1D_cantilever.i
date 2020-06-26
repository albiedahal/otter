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
#     function = '-1000*t'
#     variable = disp_y
#     boundary = 'right'
#   []
# []

# [Functions]
#   [load]
#     type = PiecewiseLinear
#     x = '0   1       2      3      4      5   6    7      8     9       10      11 12   13     14     15'
#     y = '0 0.0004 0.0008 0.0012 0.0008 0.0004 0 -0.004 -0.008 -.00012 -0.008 -0.004 0 0.0004 0.0008 0.0012'
#   []
# []

[Materials]
  [elasticity]
    type = ComputeElasticityBeam
    poissons_ratio = 0.33
    youngs_modulus = 7.310e10
  []
  [strain]
    type = ComputeIncrementalBeamStrainl
    Iy = 8.33e-6
    Iz = 8.33e-6
    area = 0.01
    rotations = 'rot_x rot_y rot_z'
    displacements = 'disp_x disp_y disp_z'
    y_orientation = '0 1 0'
  []
  [stress]
    type = ComputeBeamResultants
    block = 0
    outputs = exodus
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
    function = '0.0005'
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
    type = StressDivergenceBeaml
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
  dt = 1
  end_time = 1
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
  [disp_y2]
    type = PointValue
    point = '0.75 0 0'
    variable = disp_y
  []
  [rotation1]
    type = PointValue
    point = '1 0 0'
    variable = rot_z
  []
  [rotation2]
    type = PointValue
    point = '0.75 0 0'
    variable = rot_z
  []
  [moments_z1]
    type = PointValue
    point = '1 0 0'
    variable = moments_z
  []
  [moments_z2]
    type = PointValue
    point = '0.75 0 0'
    variable = moments_z
  []
  [forces_y1]
    type = PointValue
    point = '1 0 0'
    variable = forces_y
  []
  [forces_y2]
    type = PointValue
    point = '0.75 0 0'
    variable = forces_y
  []
  [res_forcey1]
    type = PointValue
    point = '1 0 0'
    variable = forcey
  []
  [res_forcey2]
    type = PointValue
    point = '0.75 0 0'
    variable = forcey
  []
  [res_momentz1]
    type = PointValue
    point = '1 0 0'
    variable = momentz
  []
  [res_momentz2]
    type = PointValue
    point = '0 0 0'
    variable = momentz
  []
[]

  [Outputs]
    csv = true
    exodus = true
    perf_graph = true
  []
