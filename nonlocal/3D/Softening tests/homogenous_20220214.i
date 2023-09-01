
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [bar]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 10
    ymax = 1
    zmax = 1
    nx = 10
    ny = 1
    nz = 1
  []
  # [center_element]
  #   type = ElementSubdomainIDGenerator
  #   input = bar
  #   subdomain_ids = 1
  #   element_ids = 4
  # []
[]

[AuxVariables]
  [./stress_xx]
    # order = SECOND
    # family = LAGRANGE
  [../]
  [./strain_xx]
    # order = SECOND
    # family = LAGRANGE
  [../]
    [./s_vm]
      # order = SECOND
      # family = LAGRANGE
    [../]
[]

[AuxKernels]
  [./stress_xx]
    type = RankTwoAux
    variable = stress_xx
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    # selected_qp = 1
  [../]
  [./strain_xx]
    type = RankTwoAux
    variable = strain_xx
    rank_two_tensor = total_strain
    index_i = 0
    index_j = 0
    # selected_qp = 1
  [../]
  [./mises]
      type = RankTwoScalarAux
      variable = s_vm
      rank_two_tensor = stress
     # execute_on = timestep_end
      scalar_type = VonMisesStress
      # selected_qp = 1
    [../]
[]


[Modules/TensorMechanics/Master]
  [all]
    strain = SMALL
    incremental = true
    add_variables = true
    generate_output = 'effective_plastic_strain'
  []
[]

[BCs]
  [symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [symmz]
    type = DirichletBC
    variable = disp_z
    boundary = front
    value = 0
  []
  [axial_load]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = right
    function = load
    # preset = false
  []
[]

[Functions]
  # [./load]
  #   type = PiecewiseLinear
  #   x = '0   3'
  #   y = '0   0.001'
  # [../]
  [load]
    type = ParsedFunction
    value = 'if(t<=1.0,0.0001,0.0001 + (t-1)*0.00005)'
  []
[]

[Materials]
  [stress1]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'isotropic_plasticity1'
    block = 0
  []
  # [stress2]
  #   type = ComputeMultipleInelasticStress
  #   inelastic_models = 'isotropic_plasticity2'
  #   block = 1
  # []
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 8e6
    poissons_ratio = 0
  [../]
  [./isotropic_plasticity1]
    type = IsotropicPlasticityStressUpdate
    yield_stress = 200
    hardening_constant = -5e5
    block = 0
  [../]
  # [./isotropic_plasticity2]
  #   type = IsotropicPlasticityStressUpdate
  #   yield_stress = 190
  #   hardening_constant = -0.5e6
  #   block = 1
  # [../]
[]

[Postprocessors]
  [disp]
    type = FunctionValuePostprocessor
    function = load
  []
  [stress_xx]
    type = ElementalVariableValue
    elementid = 0
    variable = stress_xx
  []
  [strain_xx]
    type = ElementalVariableValue
    elementid = 0
    variable = strain_xx
  []
  [stress_xx1]
    type = ElementalVariableValue
    elementid = 4
    variable = stress_xx
  []
  [strain_xx1]
    type = ElementalVariableValue
    elementid = 4
    variable = strain_xx
  []
  [stress_xx2]
    type = ElementalVariableValue
    elementid = 9
    variable = stress_xx
  []
  [strain_xx2]
    type = ElementalVariableValue
    elementid = 9
    variable = strain_xx
  []
  [mises]
    type = ElementalVariableValue
    elementid = 0
    variable = s_vm
  []
  [pe_eff]
    type = ElementalVariableValue
    elementid = 4
    variable = effective_plastic_strain
  []
  [mises1]
    type = ElementalVariableValue
    elementid = 4
    variable = s_vm
  []
  [pe_eff1]
    type = ElementalVariableValue
    elementid = 4
    variable = effective_plastic_strain
  []
  [mises2]
    type = ElementalVariableValue
    elementid = 9
    variable = s_vm
  []
  [pe_eff2]
    type = ElementalVariableValue
    elementid = 9
    variable = effective_plastic_strain
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
  l_tol      = 1e-8
  nl_max_its = 100
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8

  # [./TimeStepper]
  #   type = PostprocessorDT
    dt = 1
    dtmin = 0.001
  #   postprocessor = mat1
  # [../]
  start_time = 0
  end_time = 19
[]

[Outputs]
  csv=true
  exodus = true
[]
