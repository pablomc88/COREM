#ifndef RETINAOUTPUT_H
#define RETINAOUTPUT_H

/* BeginDocumentation
 * Name: RetinaOutput
 *
 * Description: Special retina module in charge of managing the retina output.
 * In particular it can (instantly) convert the output into spike times and save
 * them in a file.
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 * Author: Richard R. Carrillo. University of Granada. CITIC-UGR. Spain.
 *
 * SeeAlso: module
 */

#include <vector>
#include <string>
#include "module.h"

using namespace cimg_library;
using namespace std;

// Define spike struct to be used by RetinaOutput class to store generated spikes
struct spike_t {
    double time;
    unsigned long neuron;
};

class RetinaOutput:public module{
protected:
    // image buffers
    CImg<double> *inputImage; // Buffer used to temporally store the input values which will be converted to spikes
    
    vector<spike_t> out_spks; // Vector of retina output spikes
    string out_spk_filename; // filename (including path) to the spike output file to create
    
    // conversion parameters
    double Max_freq, Min_freq; // Max. and min. number of spikes per second that a neuron can fire
    double Input_threshold; // Minimal (sustained) input value required for a neuron to generate some output
    double Spk_freq_per_inp; // Conversion factor from input value to output spike frequency (Hz)
    
    // Last firing time and predicted firing time for each output neuron
    CImg<double> *last_spk_time, *next_spk_time;

public:
    // Constructor, copy, destructor.
    RetinaOutput(int x=1,int y=1,double temporal_step=1.0);
    RetinaOutput(const RetinaOutput& copy);
    ~RetinaOutput(void);

    // Allocate values and set protected parameters
    virtual void allocateValues();
    virtual void setX(int x){sizeX=x;}
    virtual void setY(int y){sizeY=y;}

    RetinaOutput& set_Max_freq(double max_spk_freq);
    RetinaOutput& set_Min_freq(double min_spk_freq);
    RetinaOutput& set_Input_threshold(double input_threshold);
    RetinaOutput& set_Freq_per_inp(double freq_per_inp_unit);
    RetinaOutput& set_Out_filename(string filename);

    // Get new input
    virtual void feedInput(double sim_time, const CImg<double> &new_input, bool isCurrent, int port);
    // update of state and output
    virtual void update();
    // set Parameters
    virtual bool setParameters(vector<double> params, vector<string> paramID);
    
    // This method uses the user parameter (Max_freq, Min_freq, Input_threshold and
    // Spks_per_inp) to convert the magnitude of an input pixel into an instant
    // spike firing rate.
    double inp_pixel_to_freq(double pixel_value); // TODO: static implementation

    // Save the accumulated spike activity into a file
    bool SaveFile(string spk_filename);
    
    // Get output image (y(k))
    virtual CImg<double>* getOutput();
};

#endif // RETINAOUTPUT_H
