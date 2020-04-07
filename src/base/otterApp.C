#include "otterApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"

InputParameters
otterApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  // Do not use legacy DirichletBC, that is, set DirichletBC default for preset = true
  params.set<bool>("use_legacy_dirichlet_bc") = false;

  return params;
}

otterApp::otterApp(InputParameters parameters) : MooseApp(parameters)
{
  otterApp::registerAll(_factory, _action_factory, _syntax);
}

otterApp::~otterApp() {}

void
otterApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ModulesApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"otterApp"});
  Registry::registerActionsTo(af, {"otterApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
otterApp::registerApps()
{
  registerApp(otterApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
otterApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  otterApp::registerAll(f, af, s);
}
extern "C" void
otterApp__registerApps()
{
  otterApp::registerApps();
}
