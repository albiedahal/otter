//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "otterTestApp.h"
#include "otterApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "ModulesApp.h"

InputParameters
otterTestApp::validParams()
{
  InputParameters params = otterApp::validParams();
  return params;
}

otterTestApp::otterTestApp(InputParameters parameters) : MooseApp(parameters)
{
  otterTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

otterTestApp::~otterTestApp() {}

void
otterTestApp::registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs)
{
  otterApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"otterTestApp"});
    Registry::registerActionsTo(af, {"otterTestApp"});
  }
}

void
otterTestApp::registerApps()
{
  registerApp(otterApp);
  registerApp(otterTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
otterTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  otterTestApp::registerAll(f, af, s);
}
extern "C" void
otterTestApp__registerApps()
{
  otterTestApp::registerApps();
}
