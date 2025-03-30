
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  xmax = 1
  elem_type = HEX8
[]

[AuxVariables]
  [omega]
    family = MONOMIAL
    order = CONSTANT
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
  [all]
    strain = SMALL
    incremental = true
    add_variables = true
    # volumetric_locking_correction = true
    generate_output = 'stress_xx stress_yy stress_zz strain_xx strain_yy strain_zz plastic_strain_xx vonmises_stress effective_plastic_strain'
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
  [axial_load]
    type = FunctionDirichletBC
    variable = disp_x
    # boundary = 101
    boundary = right
    function = load
  []
[]

[Functions]
  [./load]
    type = PiecewiseLinear
    x = '0   3          9        15       21       24     30        36    40    44'
    y = '0   0.000064    0.00004   0.00009    -0.00002   0.0001 -0.00005  0.00011  -0.00006 0.00012'
  [../]
[]

[Materials]
  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'plasticity'
  []
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 8e6
    poissons_ratio = 0
  [../]
  [./plasticity]
    type = SelectiveHardeningStressUpdate
    yield_stress = 200
    hardening_constant = 0.6e6
    deterioration_constant = -1e6
    gamma = 1.1
    beta = 0.95
    output_properties = viz
    outputs = exodus
  [../]
[]

[Postprocessors]
  [stress_xx]
    # type = ElementAverageValue
    type = PointValue
    point = '0 0.25 0'
    variable = stress_xx
  []
  [strain_xx]
    # type = ElementAverageValue
    type = PointValue
    point = '0 0.25 0'
    variable = strain_xx
  []
  [pe_xx]
    # type = ElementAverageValue
    type = PointValue
    point = '0 0.25 0'
    variable = plastic_strain_xx
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
  dt = 0.01
  petsc_options = '-ksp_snes_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = ' 201                hypre    boomeramg      4'
  start_time = 0
  end_time = 44
[]

[Outputs]
  csv=true
  exodus = true
  console = true
[]
