/*
 *  pif_psc_alpha.cpp
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

#include "exceptions.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "numerics.h"
#include "universal_data_logger_impl.h"
#include "lockptrdatum.h"
#include <limits>

#include "retina_interface.h"

InterfaceNEST nest::retina_interface::retina;

nest::retina_interface::Parameters_::Parameters_()
  : cell_number    (0.0),
    retina_file ("empty")
{}


nest::retina_interface::State_::State_()
{}

void nest::retina_interface::Parameters_::get(DictionaryDatum &d) const
{
  (*d)[names::port] = cell_number;
  (*d)[names::file] = retina_file;
}


void nest::retina_interface::State_::get(DictionaryDatum &d) const
{
}

void nest::retina_interface::Parameters_::set(const DictionaryDatum& d)
{
  updateValue<double_t>(d, names::port, cell_number);
  updateValue<string>(d, names::file, retina_file);
}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::retina_interface::retina_interface()
  : Node(),
    device_(),
    P_(),
    S_()
{   
}

nest::retina_interface::retina_interface(const retina_interface& n)
  : Node(n),
    device_(n.device_),
    P_(n.P_),
    S_(n.S_)
{}

/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void nest::retina_interface::init_state_(const Node& proto)
{
  const retina_interface& pr = downcast<retina_interface>(proto);

  device_.init_state(pr.device_);
  S_ = pr.S_;
  }

void nest::retina_interface::init_buffers_()
{
  device_.init_buffers();
  if(this->P_.cell_number==0.0){
        retinaInit();
  }
}

void nest::retina_interface::calibrate()
{
    device_.calibrate();
}

void nest::retina_interface::update(Time const & origin, const long_t from, const long_t to)
{

  if(this->P_.cell_number==0.0){
        retinaUpdate();
  }

  long_t start = origin.get_steps();

  CurrentEvent ce;
  for ( long_t lag = from ; lag < to ; ++lag )
    if( device_.is_active(Time::step(start+lag) ))
      {
        ce.set_current((double_t)retinaGetValue());
        network()->send(*this, ce, lag);
      }
}
