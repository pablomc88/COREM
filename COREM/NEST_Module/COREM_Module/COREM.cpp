/*
 *  COREM.cpp
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

// include necessary NEST headers
#include "config.h"
#include "network.h"
#include "model.h"
#include "dynamicloader.h"
#include "genericmodel.h"
#include "booldatum.h"
#include "integerdatum.h"
#include "tokenarray.h"
#include "exceptions.h"
#include "sliexceptions.h"
#include "nestmodule.h"
#include "connector_model_impl.h"
#include "target_identifier.h"

// include headers with your own stuff
#include "COREM.h"
#include "retina_interface.h"

// -- Interface to dynamic module loader ---------------------------------------

/*
 * The dynamic module loader must be able to find your module.
 * You make the module known to the loader by defining an instance of your
 * module class in global scope. This instance must have the name
 *
 * <modulename>_LTX_mod
 *
 * The dynamicloader can then load modulename and search for symbol "mod" in it.
 */

mynest::COREM COREM_LTX_mod;

// -- DynModule functions ------------------------------------------------------

mynest::COREM::COREM()
{
#ifdef LINKED_MODULE
  // register this module at the dynamic loader
  // this is needed to allow for linking in this module at compile time
  // all registered modules will be initialized by the main app's dynamic loader
  nest::DynamicLoaderModule::registerLinkedModule( this );
#endif
}

mynest::COREM::~COREM()
{
}

const std::string
mynest::COREM::name( void ) const
{
  return std::string( "COREM Module" ); // Return name of the module
}

const std::string
mynest::COREM::commandstring( void ) const
{
  // Instruct the interpreter to load COREM-init.sli
  return std::string( "(COREM-init) run" );
}


//-------------------------------------------------------------------------------------

void
mynest::COREM::init( SLIInterpreter* i )
{
  /* Register a neuron or device model.
     Give node type as template argument and the name as second argument.
     The first argument is always a reference to the network.
  */
//  nest::register_model< pif_psc_alpha >( nest::NestModule::get_network(), "pif_psc_alpha" );

    nest::register_model<nest::retina_interface>(nest::NestModule::get_network(),
                                            "COREM");


} // COREM::init()
