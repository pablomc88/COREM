#include "corem.h"

// C++ includes:
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <limits>

// Includes from libnestutil:
#include "numerics.h"

// Includes from nestkernel:
#include "exceptions.h"
#include "kernel_manager.h"
#include "universal_data_logger_impl.h"

// Includes from sli:
#include "dict.h"
#include "dictutils.h"
#include "doubledatum.h"
#include "integerdatum.h"
#include "lockptrdatum.h"


InterfaceNEST mynest::corem::retina;

using namespace nest;

nest::RecordablesMap< mynest::corem >
  mynest::corem::recordablesMap_;

namespace nest
{

template <>
void
RecordablesMap< mynest::corem >::create()
{
}
}

mynest::corem::Parameters_::Parameters_()
    : cell_number    (0.0),
      retina_file ("empty")
{
}

mynest::corem::State_::State_( const Parameters_& p )
{
}

void
mynest::corem::Parameters_::get( DictionaryDatum& d ) const
{
    def< double >( d, names::port, cell_number );
    def< std::string >( d, names::file, retina_file );
}

void
mynest::corem::Parameters_::set( const DictionaryDatum& d )
{
    updateValue< double >( d, names::port, cell_number );
    updateValue< std::string >( d, names::file, retina_file );

}

void
mynest::corem::State_::get( DictionaryDatum& d ) const
{
}

void
mynest::corem::State_::set( const DictionaryDatum& d,
  const Parameters_& p )
{
}

mynest::corem::Buffers_::Buffers_( corem& n )
  : logger_( n )
{
}

mynest::corem::Buffers_::Buffers_( const Buffers_&, corem& n )
  : logger_( n )
{
}

mynest::corem::corem()
  : Archiving_Node()
  , P_()
  , S_( P_ )
  , B_( *this )
{
  recordablesMap_.create();
}

mynest::corem::corem( const corem& n )
  : Archiving_Node( n )
  , P_( n.P_ )
  , S_( n.S_ )
  , B_( n.B_, *this )
{
}

mynest::corem::~corem()
{
}

void
mynest::corem::init_state_( const Node& proto )
{
  const corem& pr = downcast< corem >( proto );
  S_ = pr.S_;
}

void
mynest::corem::init_buffers_()
{
  B_.logger_.reset();

    if(this->P_.cell_number==0.0){
          retinaInit();
    }
}

void
mynest::corem::calibrate()
{
  B_.logger_.init();
}

void
mynest::corem::update( nest::Time const& origin,
                                const long from,
                                const long to )
{
  assert(
    to >= 0 && ( delay ) from < kernel().connection_manager.get_min_delay() );
  assert( from < to );

  for ( long lag = from; lag < to; ++lag )
  {
    if(this->P_.cell_number==0.0){
        retinaUpdate();
    }

    CurrentEvent ce;
    ce.set_current( (double_t)retinaGetValue() );
    kernel().event_delivery_manager.send( *this, ce, lag );

    B_.logger_.record_data( origin.get_steps() + lag );
  }
}

void
mynest::corem::handle( CurrentEvent& e )
{

}

void
mynest::corem::handle( DataLoggingRequest& e )
{
  B_.logger_.handle( e ); // the logger does this for us
}
