[Mesh]
  type = FileMesh
  file = 'Specimen_QtrModel.e'
  displacements = 'disp_x disp_y disp_z'
  # use_displaced_mesh = true
[]

[Variables]
  [disp_x]
  [../]
  [disp_y]
  [../]
  [disp_z]
  [../]
[]

[Kernels]
  [TensorMechanics]
    displacements = 'disp_x disp_y disp_z'
    # use_displaced_mesh = true
  []
[]

[BCs]
  [./Load_RE]
    type = FunctionDirichletBC
    variable = disp_x
    function = '0.005*t'
    boundary = "Right_end"
  [../]
  [./Symm_LE]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = "Left_end"
  [../]
  [./Symm_BY]
    type =DirichletBC
    variable = disp_y
    value = 0.0
    boundary = "Bottom"
  [../]
  [./Symm_LZ]
    type =DirichletBC
    variable = disp_z
    value = 0.0
    boundary = "Lateral_face"
  [../]
  [./Const_RY]
    type =DirichletBC
    variable = disp_y
    value = 0.0
    boundary = "Right_end"
  [../]

 # [./Symm_BRY]
 #   type = DirichletBC
 #   variable = disp_y
 #   value = 0.0
 #   boundary = "Bottom_right"
 # [../]
[]

[AuxVariables]
  [sdev]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [effstr]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./plastic_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e11_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
   [./stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
  [../]
  [./plastic_xx]
    type = RankTwoAux
    rank_two_tensor = plastic_strain
    variable = plastic_xx
    index_i = 0
    index_j = 0
  [../]
  [./strain_e11]
    type = RankTwoAux
    rank_two_tensor = total_strain
    index_i = 0
    index_j = 0
    variable = e11_aux
  [../]
  [sdev]
    type = RankTwoScalarAux
    variable = sdev
    rank_two_tensor = stress
    scalar_type = VonMisesStress
  []
  [streff]
    type = RankTwoScalarAux
    variable = effstr
    rank_two_tensor = plastic_strain
    scalar_type = EffectiveStrain
  []
[]

[Postprocessors]
  [./disp_xx]
    type = PointValue
    point = '0 0 0'
    variable = disp_x
  [../]
  [./sdev]
    type = PointValue
    point = '0 0 0'
    variable = sdev
  [../]
  [./s_xx]
   type = PointValue
    point = '0 0 0'
    variable = stress_xx
  [../]
 [./p_xx]
   type = PointValue
    point = '0 0 0'
    variable = plastic_xx
  [../]
 [./eps_xx]
    type = PointValue
    point = '0 0 0'
    variable = e11_aux
  [../]
[]
# [UserObjects]
#   [str]
#     type = TensorMechanicsHardeningPowerRule
#     value_0 = 4.09e8
#     epsilon0 = 0.00196
#     exponent = 0.16
#   []
#   [j2]
#     type = TensorMechanicsPlasticJ2
#     yield_strength = str
#     yield_function_tolerance = 1e-3
#     internal_constraint_tolerance = 1e-9
#   []
#[./IsotropicSD]
#    type = TensorMechanicsPlasticIsotropicSD
#    b = -0.2
#    c = -0.779422863
#    associative = true
#    yield_strength = str
#    yield_function_tolerance = 1e-5
#    internal_constraint_tolerance = 1e-9
#   use_custom_returnMap = false
#   use_custom_cto = false
#  [../]
# []

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 8e6
    poissons_ratio = 0.315
  []
  [strain]
    type = ComputeFiniteStrain
    displacements = 'disp_x disp_y disp_z'
  []
  [stress]
    # type = ComputeFiniteStrainElasticStress
    type = ComputeMultipleInelasticStress
    inelastic_models = 'kinematic_plasticity'
  []
  [./kinematic_plasticity]
    type = KinematicPlasticityStressUpdate
    # type = CombinedHardeningStressUpdatel
    yield_stress = 200
    hardening_constant = 2e6
    deterioration_constant = -0.5e6
    peak_strength = 264
    # hardening_function = hf
  [../]
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

#[Executioner]
#  type = Transient
#  solve_type = PJFNK
#  petsc_options_iname = '-pc_type'
#  petsc_options_value = 'lu'

#  num_steps = 500
#  dt = 0.02
#  dtmin = 0.005
#[]

[Executioner]
  num_steps = 500
  dt = 0.1
  type = Transient

  nl_rel_tol = 1e-6
  nl_max_its = 10
  l_tol = 1e-4
  l_max_its = 50

  solve_type = Newton
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
#  petsc_options_iname = '-pc_type -pc_hypre_type'
#  petsc_options_value = 'hypre boomerang'
#   automatic_scaling = true
[]

[Outputs]
  perf_graph = false
  exodus = true
  csv = true
[]

#[Outputs]
#  [out]
#    type = Exodus
#    csv = true
#    execute_on = 'initial timestep_end final'
#  []
#[]
