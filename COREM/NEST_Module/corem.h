/* ----------------------------------------------------------------
    COREM is implemented as a current generator in NEST. When executing COREM
    within NEST, simulation is driven by the latter one, which periodically sends
    update requests and receives data of the analog  presynaptic current of
    ganglion cells.
 * ---------------------------------------------------------------- */

#ifndef corem_H
#define corem_H

// C includes:
#include <gsl/gsl_errno.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_odeiv.h>

// Includes from nestkernel:
#include "archiving_node.h"
#include "connection.h"
#include "event.h"
#include "nest_types.h"
#include "ring_buffer.h"
#include "universal_data_logger.h"

// Corem interface
#include "../src/InterfaceNEST.h"

// Includes from sli:
#include "dictdatum.h"

namespace mynest
{

class corem : public nest::Archiving_Node
{
public:

    void retinaInit()
    {
        retina.setVerbosity(false);
        const char * retinaPath = (this->P_.retina_file).c_str();
        retina.allocateValues(retinaPath, "_output_multimeter.txt", 1.0, 0);
    }

    void retinaUpdate(){
        retina.update();
    }

    double retinaGetValue(){
        return retina.getValue(this->P_.cell_number);
    }

  corem();
  corem( const corem& );
  ~corem();

  bool has_proxies()    const { return false; }
  bool local_receiver() const { return true;  }

  using nest::Node::handle;
  using nest::Node::handles_test_event;

  nest::port send_test_event( nest::Node&, nest::port, nest::synindex, bool );

  void handle( nest::CurrentEvent& );
  void handle( nest::DataLoggingRequest& );

  nest::port handles_test_event( nest::CurrentEvent&, nest::port );
  nest::port handles_test_event( nest::DataLoggingRequest&, nest::port );

  void get_status( DictionaryDatum& ) const;
  void set_status( const DictionaryDatum& );

private:
  static InterfaceNEST retina;

  void init_state_( const Node& proto );
  void init_buffers_();
  void calibrate();

  void update( nest::Time const&, const long, const long );

  friend class nest::RecordablesMap< corem >;
  friend class nest::UniversalDataLogger< corem >;

  struct Parameters_
  {
      double cell_number;
      std::string retina_file;

      Parameters_();

      void get( DictionaryDatum& ) const;
      void set( const DictionaryDatum& );
  };

  struct State_
  {
      State_( const Parameters_& );
      void get( DictionaryDatum& ) const;
      void set( const DictionaryDatum&, const Parameters_& );
  };

  struct Buffers_
  {
      Buffers_( corem& );
      Buffers_( const Buffers_&, corem& );
      nest::UniversalDataLogger< corem > logger_;
  };

  struct Variables_
  {
  };
  Parameters_ P_;
  State_ S_;
  Variables_ V_;
  Buffers_ B_;

  static nest::RecordablesMap< corem > recordablesMap_;
};

inline nest::port
mynest::corem::send_test_event( nest::Node& target,
  nest::port receptor_type,
  nest::synindex,
  bool )
{
  nest::CurrentEvent e;
  e.set_sender( *this );
  return target.handles_test_event( e, receptor_type );
}

inline nest::port
mynest::corem::handles_test_event( nest::CurrentEvent&,
  nest::port receptor_type )
{
  if ( receptor_type != 0 )
    throw nest::UnknownReceptorType( receptor_type, get_name() );
  return 0;
}

inline nest::port
mynest::corem::handles_test_event( nest::DataLoggingRequest& dlr,
  nest::port receptor_type )
{
  if ( receptor_type != 0 )
    throw nest::UnknownReceptorType( receptor_type, get_name() );

  return B_.logger_.connect_logging_device( dlr, recordablesMap_ );
}

inline void
corem::get_status( DictionaryDatum& d ) const
{
  P_.get( d );
  S_.get( d );
  Archiving_Node::get_status( d );
  ( *d )[ nest::names::recordables ] = recordablesMap_.get_list();
}

inline void
corem::set_status( const DictionaryDatum& d )
{
  Parameters_ ptmp = P_;
  ptmp.set( d );
  State_ stmp = S_;
  stmp.set( d, ptmp );

  Archiving_Node::set_status( d );

  P_ = ptmp;
  S_ = stmp;
}

}

#endif
