# Steady-state analysis of a cantilever beam using 1D element
# The beam is made of Aluminum.
# Young's Modulus = 73.1 GPa
# Poisson's Ratio =  0.33
# Beam Dimensions = 1*0.1*0.1 m^3
# Load = 5000N at free end


[Mesh]
  [beam]
  type = GeneratedMeshGenerator
  dim = 1
  nx = 10
  xmin = 0
  xmax = 3000
  []
  [mid]
  type = ExtraNodesetGenerator
  new_boundary = mid
  coord = '1500 0 0'
  input = beam
[../]
[../]

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
#     function = '-1000*t'
#     variable = disp_y
#     boundary = 'right'
#   []
# []

# [Functions]
#   [load]
#     type = ConstantFunction
#     value = -5000
#   []
# []

[Materials]
  [elasticity]
    type = ComputeElasticityBeam
    poissons_ratio = 0.3
    youngs_modulus = 210
  []
  [strain]
    type = ComputeIncrementalBeamStrain
    Iz = 26680000
    Iy = 68480000
    area = 9600
    rotations = 'rot_x rot_y rot_z'
    displacements = 'disp_x disp_y disp_z'
    y_orientation = '0 1 0'
  []
  [stress]
    type = CoupledBeam
    block = 0
    yield_force = '8700000000 8700000000 8700000000'
    yield_moments = '196000 196000 196000'
    outputs = exodus
    output_properties = 'forces moments'
    # absolute_tolerance = 1e-4
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
  [fixx2]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0
  []
  [fixy2]
    type = DirichletBC
    variable = disp_y
    boundary = right
    value = 0
  []
  [fixz2]
    type = DirichletBC
    variable = disp_z
    boundary = right
    value = 0
  []
  [fixr21]
    type = DirichletBC
    variable = rot_x
    boundary = right
    value = 0
  []
  [fixr22]
    type = DirichletBC
    variable = rot_y
    boundary = right
    value = 0
  []
  [fixr23]
    type = DirichletBC
    variable = rot_z
    boundary = right
    value = 0
  []
  [load]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = mid
    function = '5*t'
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
  line_search = 'bt'
  dt = 1
  end_time = 5
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
    point = '1500 0 0'
    variable = disp_y
  []
  [rotation]
    type = NodalMaxValue
    boundary = right
    variable = rot_z
  []
  [forces_y]
    type = PointValue
    point = '0 0 0'
    variable = forces_y
  []
  [moments_z]
    type = PointValue
    point = '1500 0 0'
    variable = moments_z
  [../]
  # [./moments]
  #   type = ElementIntegralMaterialProperty
  #   mat_prop = moments
  # [../]
  # [forces]
  #   type = ElementIntegralMaterialProperty
  #   mat_prop = forces
  # [../]
  # [./e_xx]
  #   type = ElementIntegralMaterialProperty
  #   mat_prop = total_stretch
  # [../]
  # [./ep_xx]
  #   type = ElementIntegralMaterialProperty
  #   mat_prop = plastic_strain
  # [../]
[]

  [Outputs]
    csv = true
    exodus = true
    perf_graph = true
  []
