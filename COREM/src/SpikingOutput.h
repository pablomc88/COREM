#ifndef SPIKINGOUTPUT_H
#define SPIKINGOUTPUT_H

/* BeginDocumentation
 * Name: SpikingOutput
 *
 * Description: Special retina module in charge of generating retina output action potentials.
 * In particular it can instantly convert the retina output into spike times and
 * finally save them in a file.
 * This module supports deterministics or stochastic spikes times.
 * A piecewise-stationary gamma process is implemented to generate stochastic spikes times.
 * Therefore, the generated inter-spike intervals (ISI) are drawn from the gamma distribution.
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
    // Predicted firing time for each output neuron (in seconds)
    CImg<double> *next_spk_time; // This time value is relative to the next sim. slot start time and unwarped
    // Last firing time for each output neuron (in seconds). It is used only to check the refractory period
    CImg<double> *last_spk_time; // This time value is absolute and warped    
    // Current refractory period for each output neuron (in seconds). It is used only to check the refractory period
    CImg<double> *curr_ref_period; // This time value is change if Min_period_std_dev is not 0 

    default_random_engine rand_gen; // For generating random numbers: neuron output random noise, random states and refractory period
    normal_distribution<double> norm_dist; // For introducing noise in neuron random max. firing rate
    gamma_distribution<double> gam_dist; // For generating random spike times
    uniform_real_distribution<double> unif_dist; // For generating neuron random init states

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
    double First_spk_delay; // It specifies the delay of the first spike of each neuron in proportion to the first firing period. If this value is 0, all neurons start firing at time 0. If it is 1, all neurons waits for the first firing period before firing (default behaviour)

public:
    // Constructor, copy, destructor.
    SpikingOutput(int x=1, int y=1, double temporal_step=1.0, string output_filename="");
    SpikingOutput(const SpikingOutput& copy);
    ~SpikingOutput(void);

    // Allocate values and set protected parameters
    virtual bool allocateValues();

    // Set spiking output parameters
    bool set_Min_period(double min_spk_per);
    bool set_Longest_sustained_period(double max_spk_per);
    bool set_Input_threshold(double input_threshold);
    bool set_Freq_per_inp(double freq_per_inp_unit);
    bool set_Spike_dist_shape(double shape_val);
    bool set_Min_period_std_dev(double sigma_val);
    bool set_Start_time(double start_time);
    bool set_End_time(double end_time);
    bool set_Random_init(double rnd_init);
    bool set_First_spk_delay(double spk_delay);
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
    
    // Apply refractory period effect to a new spike time
    double apply_ref_period(double new_spk_time, double last_spk_time, double cur_min_period);

    // Renew the value of the refractory periodof a neuron
    void renew_ref_period_val(CImg<double>::iterator curr_ref_period_it);

    // This method basically gerates spike times during current simulation time slot for one neuron.
    // For this, this method calculates the firing period (ISI) corresponding to the current input and
    // generates one spike after each period.
    // The spike times are stochastic or deterministic depending on the parameter Spike_dist_shape.
    // To simulate a non-stationary processes (since input may change in each simulation slot) first
    // firing periods are drawn from a stationary gamma process (corresponding to a constant input of 1)
    // and then these periods are "warped" according to the actual neuron input. That is, the initial
    // periods are expanded or contracted to match the desired mean output frequency. For a detailed
    // explanaton search for "warp" in:
    // Nawrot, M. P., Boucsein, C., Molina, V. R., Riehle, A., Aertsen, A., & Rotter, S. (2008).
    // Measurement of variability dynamics in cortical spike trains. Journal of neuroscience methods,
    // 169(2), 374-390.
    // Method precondition and postcondition:
    // next_spk_time must be neither infinite nor negative
    vector<spike_t> stochastic_spike_generation(unsigned long out_neu_idx, double input_val, CImg<double>::iterator next_spk_time_it, CImg<double>::iterator last_spk_time_it, CImg<double>::iterator curr_ref_period_it);

    // This method randomizes the state of the spike generator for all the outputs so that
    // each neuron will start firing at random times (from 0 to the initial firing period)
    // That is, it makes the delay of the first spike random in inverse Random_init proportion of the first period
    void randomize_state();

    // This method initizlizes the state of the all the outputs (neurons)
    void initialize_state();
    
    // Save the accumulated spike activity into a file
    bool SaveFile(string spk_filename);
    
    // Get output image (y(k))
    virtual CImg<double>* getOutput();
    
    // Returns false to indicate that this class performs computation
    virtual bool isDummy();
};

#endif // SPIKINGOUTPUT_H
