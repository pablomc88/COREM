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
    
    // Last firing period and predicted firing time for each output neuron (in seconds)
    CImg<double> *last_firing_period, *next_spk_time;

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
    // generates 1 spike after each period.
    // The extra complexity arises when calculating the time of the first spike of this series.
    // If the previous input is the same as the current one, it is easy, we use the same period to space
    // the spike series. But when the previous input is different from the current one, we calculate the
    // time of the next spike taking into account: the firing period correspondind to the last input,
    // the time of the spike that would be fired is the last input continued during this sim. slot, the 
    // firing period correspondind to the current input, and the start of the current sim. slot.
    // Considering these variables we calculate the ISI between the last series spike and the new series
    // first spike. We do it in a way that is consistent with the previous and the current input.
    // The intuitive interpretation of this calculation is that if a spike of the last series occurred
    // very recently, it pushes forward the first spike of the current series (in order not to generate
    // a very small ISI), especially if the firing period of the current input is large orthe firing
    // period of the last input was small.
    // The final outcome is that the resulting complete series of spikes is relatively smooth (if the
    // input does not changes abruptly, of course) and it does not depend on the sim. slot times.
    // One key point for this caculation is to consider what ISI we get when we get a zero input.
    // In this implementation we consider that a zero input normally does not reset the pushing effect
    // of the last elicited spike, however, another convection could be followed.
    // Method precondition and postcondition: last_firing_period must not be zero,
    // next_spk_time must not be infinite and next_spk_time higher or equals than current tslot_start.
    vector<spike_t> deterministic_spike_generation(unsigned long out_neu_idx, double input_val, CImg<double>::iterator last_firing_period_it, CImg<double>::iterator next_spk_time_it);
    
    vector<spike_t> stochastic_spike_generation(unsigned long out_neu_idx, double input_val, CImg<double>::iterator last_firing_period_it, CImg<double>::iterator next_spk_time_it);

    // This method initializes randomly the state of the spike generator for all the outputs so that
    // each neuron will start firing at random times (from 0 to the initial firing period)
    void randomize_state();

    // Save the accumulated spike activity into a file
    bool SaveFile(string spk_filename);
    
    // Get output image (y(k))
    virtual CImg<double>* getOutput();
    
    // Returns false to indicate that this class performs computation
    virtual bool isDummy();
};

#endif // SPIKINGOUTPUT_H
