#ifndef PIF_PSC_ALPHA_H
#define PIF_PSC_ALPHA_H


#include "nest.h"
#include "event.h"
#include "node.h"
#include "connection.h"
#include "stimulating_device.h"

#include <string>
#include <iostream>
#include "../../src/InterfaceNEST.h"

using namespace std;

namespace nest{

  class Network;

class retina_interface: public Node
{

public:

    void retinaInit()
    {
        retina.reset(1,1,1.0,1);
        const char * retinaPath = (this->P_.retina_file).c_str();
        const char * outputFile = "temporal";
        double outputfactor = 1.0;

        retina.allocateValues(retinaPath,outputFile,outputfactor,0);

    }

    void retinaUpdate(){
        retina.update();
    }

    double retinaGetValue(){
        return retina.getValue(this->P_.cell_number);
    }

  retina_interface();
  retina_interface(const retina_interface&);

  bool has_proxies() const {return false;}
  port send_test_event( Node&, rport, synindex, bool );

  void get_status(DictionaryDatum &) const;
  void set_status(const DictionaryDatum &) ;

private:
  static InterfaceNEST retina;

  void init_state_(const Node&);
  void init_buffers_();
  void calibrate();

  void update(Time const &, const long_t, const long_t);

  // ------------------------------------------------------------

  struct Parameters_ {
    double cell_number;
    string retina_file;

    Parameters_();  //!< Sets default parameter values

    void get(DictionaryDatum&) const;  //!< Store current values in dictionary
    void set(const DictionaryDatum&);  //!< Set values from dicitonary
  };

  // ------------------------------------------------------------


  struct State_ {
    State_();  //!< Sets default parameter values

    void get(DictionaryDatum&) const;  //!< Store current values in dictionary
  };


  StimulatingDevice<CurrentEvent> device_;
  Parameters_ P_;
  State_      S_;
};


inline port
retina_interface::send_test_event( Node& target, rport receptor_type, synindex syn_id, bool )
{
  device_.enforce_single_syn_type( syn_id );

  CurrentEvent e;
  e.set_sender( *this );

  return target.handles_test_event( e, receptor_type );
}

inline
  void retina_interface::get_status(DictionaryDatum &d) const
{
  P_.get(d);
  S_.get(d);
  device_.get_status(d);
}

inline
  void retina_interface::set_status(const DictionaryDatum &d)
{
    Parameters_ ptmp = P_;
  ptmp.set(d);
  device_.set_status(d);
  P_ = ptmp;
}


} // namespace

#endif /* #ifndef PIF_PSC_ALPHA_H */
