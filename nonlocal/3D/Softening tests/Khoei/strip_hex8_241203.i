
[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = FileMesh
  file = strip_hex8_241203.e
[]


[AuxVariables]
  # [./stress_xx]
  #   # order = SECOND
  #   # family = LAGRANGE
  # [../]
  # [./strain_xx]
  #   # order = SECOND
  #   # family = LAGRANGE
  # [../]
  # [./epeff]
  #   # order = SECOND
  #   # family = LAGRANGE
  # [../]
[]

[AuxKernels]
  # [./stress_xx]
  #   type = RankTwoAux
  #   variable = stress_xx
  #   rank_two_tensor = stress
  #   index_i = 0
  #   index_j = 0
  #   # selected_qp = 1
  # [../]
  # [./strain_xx]
  #   type = RankTwoAux
  #   variable = strain_xx
  #   rank_two_tensor = total_strain
  #   index_i = 0
  #   index_j = 0
  #   # selected_qp = 1
  # [../]
  # [./epeff]
  #     type = RankTwoScalarAux
  #     variable = epeff
  #     rank_two_tensor = plastic_strain
  #    # execute_on = timestep_end
  #     scalar_type = EffectiveStrain
  #     # selected_qp = 1
  #   [../]
[]


[Modules/TensorMechanics/Master]
  [all]
    strain = SMALL
    incremental = true
    add_variables = true
    generate_output = 'stress_xx strain_xx effective_plastic_strain vonmises_stress'
  []
[]

[BCs]
  [symmx]
    type = DirichletBC
    variable = disp_x
    boundary = 10
    value = 0
  []
  [symmy]
    type = DirichletBC
    variable = disp_y
    boundary = 1000
    value = 0
  []
  [symmz]
    type = DirichletBC
    variable = disp_z
    boundary = 11
    value = 0
  []
  [axial_load]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 12
    function = load
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
    value = '0.1*t'
  []
[]

[Materials]
  [stress1]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'isotropic_plasticity1'
    block = 1
  []
  [stress2]
    type = ComputeMultipleInelasticStress
    inelastic_models = 'isotropic_plasticity2'
    block = 2
  []
  [./elasticity_tensor1]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 4000
    poissons_ratio = 0.49
    block = '1'
  [../]
  [./elasticity_tensor2]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 4000
    poissons_ratio = 0.3
    block = '2'
  [../]
  [./isotropic_plasticity1]
    type = IsotropicPlasticityStressUpdate
    yield_stress = 100
    hardening_constant = -120
    block = 1
  [../]
  [./isotropic_plasticity2]
    type = IsotropicPlasticityStressUpdate
    yield_stress = 98
    hardening_constant = -120
    block = 2
  [../]
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
  # [mises]
  #   type = ElementalVariableValue
  #   elementid = 0
  #   variable = s_vm
  # []
  [pe_eff]
    type = ElementalVariableValue
    elementid = 4
    variable = effective_plastic_strain
  []
  # [mises1]
  #   type = ElementalVariableValue
  #   elementid = 4
  #   variable = s_vm
  # []
  [pe_eff1]
    type = ElementalVariableValue
    elementid = 4
    variable = effective_plastic_strain
  []
  # [mises2]
  #   type = ElementalVariableValue
  #   elementid = 9
  #   variable = s_vm
  # []
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
  line_search = none

  l_max_its  = 50
  l_tol      = 1e-8
  nl_max_its = 25
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-8

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '  201               hypre    boomeramg      10'


  # [./TimeStepper]
  #   type = PostprocessorDT
    dt = 0.5
    dtmin = 0.001
  #   postprocessor = mat1
  # [../]
  start_time = 0
  end_time = 40
[]

[Outputs]
  csv=true
  exodus = true
[]
