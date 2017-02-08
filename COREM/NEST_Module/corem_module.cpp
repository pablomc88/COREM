/*
 *  corem_module.cpp
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "corem_module.h"

// Generated includes:
#include "config.h"

// include headers with your own stuff
#include "corem.h"

// Includes from nestkernel:
#include "connection_manager_impl.h"
#include "connector_model_impl.h"
#include "dynamicloader.h"
#include "exceptions.h"
#include "genericmodel.h"
#include "genericmodel_impl.h"
#include "kernel_manager.h"
#include "model.h"
#include "model_manager_impl.h"
#include "nestmodule.h"
#include "target_identifier.h"

// Includes from sli:
#include "booldatum.h"
#include "integerdatum.h"
#include "sliexceptions.h"
#include "tokenarray.h"

// -- Interface to dynamic module loader ---------------------------------------

#if defined( LTX_MODULE ) | defined( LINKED_MODULE )
mynest::corem_module corem_module_LTX_mod;
#endif
// -- DynModule functions ------------------------------------------------------

mynest::corem_module::corem_module()
{
#ifdef LINKED_MODULE
  // register this module at the dynamic loader
  // this is needed to allow for linking in this module at compile time
  // all registered modules will be initialized by the main app's dynamic loader
  nest::DynamicLoaderModule::registerLinkedModule( this );
#endif
}

mynest::corem_module::~corem_module()
{
}

const std::string
mynest::corem_module::name( void ) const
{
  return std::string( "COREM" ); // Return name of the module
}

const std::string
mynest::corem_module::commandstring( void ) const
{
  // Instruct the interpreter to load corem_module-init.sli
  return std::string( "(corem_module-init) run" );
}

//-------------------------------------------------------------------------------------

void
mynest::corem_module::init( SLIInterpreter* i )
{
  /* Register a neuron or device model.
     Give node type as template argument and the name as second argument.
  */
  nest::kernel().model_manager.register_node_model< corem >(
    "corem" );

} // corem_module::init()
