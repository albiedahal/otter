[Mesh]
  [generated_mesh]
    type = FileMeshGenerator
    file = ct.e
  []
  [fix]
    type = BoundingBoxNodeSetGenerator
    input = generated_mesh
    bottom_left = '50 0 0'
    top_right = '50 15 25'
    new_boundary = 54
  []
  second_order = false
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
  order = FIRST
  family = LAGRANGE
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Variables]
[]

[AuxVariables]
  [saved_x]
  []
  [saved_y]
  []
  [saved_z]
  []
[]

[Functions]
  [push_up]
    type = PiecewiseLinear
    x = '0   3'     #9     15      21     24    30    36'
    y = '0   2100'  # -60    70     -70     80  -80   90'
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    add_variables = true
    strain = FINITE
    generate_output = 'stress_xx stress_xy stress_xz stress_yy vonmises_stress'
    save_in = 'saved_x saved_y saved_z'
  []
[]

[BCs]
  [./Pressure]
    [Side1]
      boundary = 24
      function = push_up
    []
  []
  [botx]
    type = DirichletBC
    variable = disp_x
    boundary = '54'
    value = 0.0
  []
  [disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = 54
    value = 0
  []
  [boty]
    type = DirichletBC
    variable = disp_y
    boundary = 53
    value = 0.0
  []
[]

[Materials]
  [radial_return_stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'bilin'
  []
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 156000
  []
  [bilin]
    type = Bilin1
    yield_stress = 170
    hardening_constant = 700
    deterioration_constant = -500
    peak_strength = 190
  []
[]

[Postprocessors]
  [react_y]
    type = NodalSum
    variable = saved_y
    boundary = 53
  []
  [lld]
    type = PointValue
    variable = disp_y
    point = '0 24 0'
  []
  [d53]
    type = NodalMaxValue
    boundary = 53
    variable = disp_y
  []
[]

[Executioner]
  type = Transient
  solve_type = PJFNK

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  line_search = none

  l_max_its = 15
  nl_max_its = 20
  start_time = 0
  end_time = 3
[]

[Outputs]
  csv = true
  exodus = true
  perf_graph = true
  [console]
    type = Console
    max_rows = 5
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]
