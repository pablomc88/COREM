#ifndef SPIKINGOUTPUT_H
#define SPIKINGOUTPUT_H

/* BeginDocumentation
 * Name: SpikingOutput
 *
 * Description: Special retina module in charge of generating action potentials as output.
 * In particular it can instantly (without delay) convert the retina output into spike times and
 * finally save them in a file.
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 * Author: Richard R. Carrillo. University of Granada. CITIC-UGR. Spain.
 *
 * SeeAlso: module
 */

#include <vector>
#include <string>
#include <random>
#include "module.h"

using namespace cimg_library;
using namespace std;

// Define spike struct to be used by SpikingOutput class to store generated spikes
struct spike_t {
    double time;
    unsigned long neuron;
};

class SpikingOutput:public module{
    // External (parameters) variables are in millisecond. Internal class calculations are done in whole units (seconds).
protected:
    // image buffers
    CImg<double> *inputImage; // Buffer used to temporally store the input values which will be converted to spikes

    vector<spike_t> out_spks; // Vector of retina output spikes
    string out_spk_filename; // filename (including path) to the spike output file to create
    
    double Start_time, End_time; // These recording parameters define the simulation time interval when the images must be saved (in milliseconds)
    
    // These parameter select which pixels will be condiered, thus restricting the number of outputs as well
    size_t First_inp_ind, Inp_ind_inc; // Index of the first pixel (from 0), Increment the get the next pixel index
    unsigned long Total_inputs; // and total number of pixels to consider

    // parameters for conversion from input magnitude to firing rate
    double Min_period; // Minimum time in milliseconds allowed to elape between two consecutive spikes of a neuron (min. ISI or refractory period)
    double Longest_sustained_period; // Firing period in milliseconds when the input is equal to the input threshold (offset firing frequency)
    double Input_threshold; // Minimal (sustained) input value required for a neuron to generate sustained output
    double Spk_freq_per_inp; // Conversion factor from input value to output spike frequency (Hz):
    // firing_period_ms = 1 / (pixel_value - Input_threshold)*(Spk_freq_per_inp/1000) + Lower_sustained_period_ms
    double Spike_dist_shape; // When this value is specified and different from infinity, inter-spike intervals are randomly drawn from a gamma distribution whose shape is specified by this parameter. If this parameter is not specified, firing period is stochastic. The higher this value is, the lower standard deviation
    double Min_period_std_dev; // Standard deviation in milliseconds of a normal distribition with mean Min_period from which min. firing periods are drawn. When this value is equal to 0, min. firing periods are fixed
    double Random_init; // If differnt from 0, it configures the initial state randomly, so that the starting firing phase is uniformly random between 1 and (1-Random_init)*first_firing_period
    // Since the spike time generation is not impemented as a real non-homogeneous random process, Spike_std_dev is not exactly
    // the standard deviation of the generated spike times, but the standar deviation of the distribution used to generate these spikes.
    default_random_engine rand_gen; // For generating random numbers: neuron output random noise, random states and refractory period
    normal_distribution<double> norm_dist; // For introducing noise in neuron random max. firing rate
    gamma_distribution<double> gam_dist; // For generating random spike times
    uniform_real_distribution<double> unif_dist; // For generating neuron random init states
    
    // Predicted firing time for each output neuron (in seconds)
    CImg<double> *next_spk_time; // This time value is relative to the next sim. slot start time

public:
    // Constructor, copy, destructor.
    SpikingOutput(int x=1, int y=1, double temporal_step=1.0, string output_filename="");
    SpikingOutput(const SpikingOutput& copy);
    ~SpikingOutput(void);

    // Allocate values and set protected parameters
    virtual bool allocateValues();

    bool set_Min_period(double min_spk_per);
    bool set_Longest_sustained_period(double max_spk_per);
    bool set_Input_threshold(double input_threshold);
    bool set_Freq_per_inp(double freq_per_inp_unit);
    bool set_Spike_dist_shape(double shape_val);
    bool set_Min_period_std_dev(double sigma_val);
    bool set_Start_time(double start_time);
    bool set_End_time(double end_time);
    bool set_Random_init(double rnd_init);
    bool set_First_inp_ind(double first_input);
    bool set_Inp_ind_inc(double input_inc);
    bool set_Total_inputs(double num_inputs);

    // Get new input
    virtual void feedInput(double sim_time, const CImg<double> &new_input, bool isCurrent, int port);
    // update of state and generate output spikes for all neurons during current sim. slot
    virtual void update();
    // set Parameters
    virtual bool setParameters(vector<double> params, vector<string> paramID);
    
    // This method uses the user parameter (Max_freq, Min_freq, Input_threshold and
    // Spks_per_inp) to convert the magnitude of an input pixel into an instant
    // spike firing rate.
    double inp_pixel_to_period(double pixel_value);
    
    // This method basically gerates spike times during current simulation time slot for one neuron.
    // For this, this method calculates the firing period (ISI) corresponding to the current input and
    // generates one spike after each period.
    // The spike times are stochastic or deterministic depending on the parameter Spike_dist_shape.
    // Method precondition and postcondition:
    // next_spk_time must be neither infinite nor negative
    vector<spike_t> stochastic_spike_generation(unsigned long out_neu_idx, double input_val, CImg<double>::iterator next_spk_time_it);

    // This method randomize the state of the spike generator for all the outputs so that
    // each neuron will start firing at random times (from 0 to the initial firing period)
    // That is, it makes the delay of the first spike random in inverse Random_init proportion of the first period
    void randomize_state();

    // This method randomize the state of the spike generator for all the outputs
    void initialize_state();
    // Save the accumulated spike activity into a file
    bool SaveFile(string spk_filename);
    
    // Get output image (y(k))
    virtual CImg<double>* getOutput();
    
    // Returns false to indicate that this class performs computation
    virtual bool isDummy();
};

#endif // SPIKINGOUTPUT_H
