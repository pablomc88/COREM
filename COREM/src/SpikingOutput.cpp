#include <algorithm> // for std::sort
#include <iostream>

// For file writing:
#include <fstream>
#include <iterator>
#include <string>

#include <random> // To generate neuron noise
#include <ctime> // To log the current time in output spike file
#include <limits>
#include <iomanip> // for std::setprecision 

#include "SpikingOutput.h"

SpikingOutput::SpikingOutput(int x, int y, double temporal_step, string output_filename):module(x,y,temporal_step){
    // input-to-spike-time conversion parameters (default values)
    Min_period=0.0; // Max. firing frequency is inifite Hz
    Longest_sustained_period=numeric_limits<double>::infinity(); // Neuron start firing from 0Hz
    Input_threshold=0; // Neuron start firing from just above 0 input
    Spk_freq_per_inp=1; // Neuron fires 1 spike/second (1Hz) when input is 1
    Spike_dist_shape=numeric_limits<double>::infinity(); // Non-stochastic firing rate
    Min_period_std_dev=0.0; // Hard refractory period
    if(output_filename.compare("") != 0) 
        out_spk_filename=output_filename;
    else
        out_spk_filename="results/spikes.spk";

    // Save all input images by default
    Start_time=0.0;
    End_time=numeric_limits<double>::infinity();

    // Process al input pixels by default
    First_inp_ind=0;
    Inp_ind_inc=1;
    Total_inputs=~0UL; // Practcally infinite 
    
    Random_init=0.0; // Same initial state for all neurons
    
    First_spk_delay=1.0; // Neurons start firing after the complete first spiking period
    
    normal_distribution<double>::param_type init_norm_dist_params(Min_period/1000.0, Min_period_std_dev/1000.0); // Random numbers from a Gaussian distribution of mean=Min_period/1000.0 and sigma=Min_period_std_dev/1000.0 seconds
    norm_dist.param(init_norm_dist_params); // Set params of normal distribution
    
    uniform_real_distribution<double>::param_type init_unif_dist_params(0.0, 1.0); // Random numbers distributed uniformly between 0 and 1 (not included)
    unif_dist.param(init_unif_dist_params); // Set params of uniform distribution

    // Gamma params set in allocateValues()

    // Input buffer
    inputImage=new CImg<double> (sizeY, sizeX, 1, 1, 0);

    // Internal state variables: initial value 
    next_spk_time=new CImg<double>(sizeY, sizeX, 1, 1, First_spk_delay); // Next predicted spike time = 1 period
    last_spk_time=new CImg<double>(sizeY, sizeX, 1, 1, -numeric_limits<double>::infinity()); // Last spike time = -infinity
    curr_ref_period=new CImg<double>(sizeY, sizeX, 1, 1, Min_period/1000.0); // Current refractory period
}

SpikingOutput::SpikingOutput(const SpikingOutput &copy):module(copy){
    Min_period = copy.Min_period;
    Longest_sustained_period = copy.Longest_sustained_period;
    Input_threshold = copy.Input_threshold;
    Spk_freq_per_inp = copy.Spk_freq_per_inp;
    Spike_dist_shape = copy.Spike_dist_shape;
    Min_period_std_dev = copy.Min_period_std_dev;
    out_spk_filename = copy.out_spk_filename;
    Random_init = copy.Random_init;
    First_spk_delay = copy.First_spk_delay;
    norm_dist = copy.norm_dist;
    unif_dist = copy.unif_dist;
    gam_dist = copy.gam_dist;

    inputImage=new CImg<double>(*copy.inputImage);
    next_spk_time=new CImg<double>(*copy.next_spk_time);
    last_spk_time=new CImg<double>(*copy.last_spk_time);
    curr_ref_period=new CImg<double>(*copy.curr_ref_period);
}

SpikingOutput::~SpikingOutput(){
    // Save generated spikes before destructing the object
    cout << "Saving output spike file: " << out_spk_filename << "... " << flush;
    cout << (SaveFile(out_spk_filename)?"Ok":"Fail") << endl;
    
    delete inputImage;
    delete next_spk_time;
    delete last_spk_time;
    delete curr_ref_period;
}

//------------------------------------------------------------------------------//

void SpikingOutput::randomize_state(){
    CImg<double>::iterator next_spk_time_it = next_spk_time->begin();

    while(next_spk_time_it < next_spk_time->end()){ // For every spiking output
        // To randomize the state of each output, we set the last next_spk_time
        // to the next firing reduced to a random percentage defined by Random_init.
        *next_spk_time_it = (1.0 - Random_init*unif_dist(rand_gen)) * *next_spk_time_it; // random number in the interval [0,1) seconds from unif. dist. multiplied by previous initial firing period
        next_spk_time_it++;
    }
}

//------------------------------------------------------------------------------//

void SpikingOutput::initialize_state(){
    CImg<double>::iterator next_spk_time_it = next_spk_time->begin();
    CImg<double>::iterator curr_ref_period_it = curr_ref_period->begin();

    while(next_spk_time_it < next_spk_time->end()){ // For every spiking output
        double first_firing_period;
        // To initialize the state of each output, we set the last next_spk_time
        // to the next firing period.
        
        // Determine firing period in the "unwarped" time slot
        if(isfinite(Spike_dist_shape)) // Select stochastic or deterministic spike times
            first_firing_period = gam_dist(rand_gen);
        else // Spike_dist_shape is infinite (not specified), so we do not use stochasticity
            first_firing_period = 1; // 1Hz is the firing freq. in a "unwarped" time slot

        *next_spk_time_it = first_firing_period * First_spk_delay; // Anticipate (if First_spk_delay < 1) or postpone (if First_spk_delay > 1) first spike time according to First_spk_delay
        
        // Set refractory eriod for each neuron
        if(Min_period_std_dev == 0.0) // If fixed refractory period:
            *curr_ref_period_it = Min_period/1000.0;
        else
            *curr_ref_period_it = norm_dist(rand_gen);
            
        next_spk_time_it++;
        curr_ref_period_it++;
    }
}

//------------------------------------------------------------------------------//

bool SpikingOutput::allocateValues(){
    module::allocateValues(); // Use the allocateValues() method of the base class

    // Set parameters of distributions for random number generation
    normal_distribution<double>::param_type init_norm_dist_params(Min_period/1000.0, Min_period_std_dev/1000.0); // (mean=Min_period/1000.0, sigma=Min_period_std_dev/1000.0 seconds)
    norm_dist.param(init_norm_dist_params); // Set initial params of normal distribution

    gamma_distribution<double>::param_type init_gam_dist_params(Spike_dist_shape, 1.0/Spike_dist_shape); // Parameters of gamma distribution for spike: gam_k (alpha), gam_theta (beta)
    gam_dist.param(init_gam_dist_params); // Set params of uniform distribution

    // Resize initial image buffers
    inputImage->assign(sizeY, sizeX, 1, 1, 0.0);
    next_spk_time->assign(sizeY, sizeX, 1, 1, First_spk_delay);
    last_spk_time->assign(sizeY, sizeX, 1, 1, -numeric_limits<double>::infinity());
    curr_ref_period->assign(sizeY, sizeX, 1, 1, Min_period/1000.0);

    initialize_state(); // Set ref. period and unwarped first spike time

    if(Random_init != 0.0) // If parameter Random_init is differnt from 0, init the state of outputs randomly
        randomize_state();
    return(true);
}

bool SpikingOutput::set_Min_period(double min_spk_per){
    bool ret_correct;
    if (min_spk_per>=0) {
        Min_period = min_spk_per;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct); // Return value is used to inform the caller function if value has been set
}

bool SpikingOutput::set_Longest_sustained_period(double max_spk_per){
    bool ret_correct;
    if (max_spk_per>=0) {
        Longest_sustained_period = max_spk_per;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SpikingOutput::set_Input_threshold(double input_threshold){
    Input_threshold = input_threshold;
    return(true);
}

bool SpikingOutput::set_Freq_per_inp(double freq_per_inp_unit){
    Spk_freq_per_inp = freq_per_inp_unit;
    return(true);
}

bool SpikingOutput::set_Spike_dist_shape(double shape_val){
    bool ret_correct;
    if (shape_val>0) {
        Spike_dist_shape = shape_val;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SpikingOutput::set_Min_period_std_dev(double std_dev_val){
    bool ret_correct;
    if (std_dev_val>=0) {
        Min_period_std_dev = std_dev_val;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SpikingOutput::set_Start_time(double start_time){
    bool ret_correct;
    if (start_time>=0) {
        Start_time = start_time;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SpikingOutput::set_End_time(double end_time){
    bool ret_correct;
    if (end_time>=0) {
        End_time = end_time;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SpikingOutput::set_Random_init(double rnd_init){
    Random_init = rnd_init;
    return(true);
}

bool SpikingOutput::set_First_spk_delay(double spk_delay){
    bool ret_correct;
    if (spk_delay>=0) {
        First_spk_delay = spk_delay;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SpikingOutput::set_First_inp_ind(double first_input){
    bool ret_correct;
    if (first_input>=0) {
        First_inp_ind = first_input;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SpikingOutput::set_Inp_ind_inc(double input_inc){
    bool ret_correct;
    if (input_inc>=0) {
        Inp_ind_inc = input_inc;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SpikingOutput::set_Total_inputs(double num_inputs){
    bool ret_correct;
    if (num_inputs>=0) {
        Total_inputs = num_inputs;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

//------------------------------------------------------------------------------//

bool SpikingOutput::setParameters(vector<double> params, vector<string> paramID){

    bool correct = true;

    for (vector<double>::size_type i = 0;i<params.size() && correct;i++){
        const char * s = paramID[i].c_str();

        if (strcmp(s,"Min_period")==0){
            correct = set_Min_period(params[i]);
        } else if (strcmp(s,"Longest_sustained_period")==0){
            correct = set_Longest_sustained_period(params[i]);
        } else if (strcmp(s,"Input_threshold")==0){
            correct = set_Input_threshold(params[i]);
        } else if (strcmp(s,"Freq_per_inp")==0){
            correct = set_Freq_per_inp(params[i]);
        } else if (strcmp(s,"Spike_dist_shape")==0){
            correct = set_Spike_dist_shape(params[i]);
        } else if (strcmp(s,"Min_period_std_dev")==0){
            correct = set_Min_period_std_dev(params[i]);
        } else if (strcmp(s,"Start_time")==0){
            correct = set_Start_time(params[i]);
        } else if (strcmp(s,"End_time")==0){
            correct = set_End_time(params[i]);
        } else if (strcmp(s,"Random_init")==0){
            correct = set_Random_init(params[i]);
        } else if (strcmp(s,"First_spk_delay")==0){
            correct = set_First_spk_delay(params[i]);
        } else if (strcmp(s,"First_inp_ind")==0){
            correct = set_First_inp_ind(params[i]);
        } else if (strcmp(s,"Inp_ind_inc")==0){
            correct = set_Inp_ind_inc(params[i]);
        } else if (strcmp(s,"Total_inputs")==0){
            correct = set_Total_inputs(params[i]);
        } else
            correct = false;
    }
    return correct;
}

//------------------------------------------------------------------------------//

void SpikingOutput::feedInput(double sim_time, const CImg<double>& new_input, bool isCurrent, int port){
    // Ignore port type and copy input image
    if(simTime >= Start_time && simTime+step <= End_time) // Check if the user wants to generate output for the image at current time
        *inputImage = new_input;
    else
        inputImage->assign(); // Reset the input image to an empty image so that no spikes are generated during this sim. time step

    // Update the current simulation time
    simTime = sim_time;
}

//------------------------------------------------------------------------------//

double SpikingOutput::inp_pixel_to_period(double pixel_value){
    double firing_period_sec; // Calculated firing period in seconds
    double max_period_sec; // Longest firind period: Offset firing period added to resultant firing period

    // Convert periods from ms to s:
    max_period_sec = Longest_sustained_period/1000.0;


    if(pixel_value < Input_threshold) // If value under threshold, output activity is 0Hz
        firing_period_sec = numeric_limits<double>::infinity();
    else
        firing_period_sec = 1.0 / ((pixel_value-Input_threshold)*Spk_freq_per_inp + 1.0/max_period_sec);

    return(firing_period_sec);
}

// All times in this fn are relative to current sim. slot start time
// cur_min_period is the current (used for this calculation) firing period in seconds (refractory period)
double SpikingOutput::apply_ref_period(double new_spk_time, double last_spk_time, double cur_min_period){
    double ref_spk_time; // New spike time with refractory period applied
    double new_firing_period; // New inter-spike period

    new_firing_period = new_spk_time - last_spk_time;
    
    // Firing rate is saturated
    if(new_firing_period < cur_min_period) // In refractory period
        ref_spk_time = last_spk_time + cur_min_period;
    else // Not in refractory period
        ref_spk_time = new_spk_time;

    if(ref_spk_time < 0.0) // Spike time must be always positive or 0
        ref_spk_time = 0;

    return(ref_spk_time);
}

void SpikingOutput::renew_ref_period_val(CImg<double>::iterator curr_ref_period_it){
    if(Min_period_std_dev > 0.0) // Add Gaussian white noise to the ref. period: Stochastic Min_period limit chosen
        *curr_ref_period_it = norm_dist(rand_gen); // We only renew the refractory period time after the neuron fires. Since only one realization of the period is considered the distribution std. dev. does not need to be adjusted, as it is in DOI:10.1523/JNEUROSCI.3305-05.2005
    // Else: no noise in freq limit.: fixed limit already set
}

// time of next spike (*next_spk_time_it) is relative to slot start time and corresponding to a input rate of value 1.
// *next_spk_time_it specifies the spike time in a "unwarped" time slot (time is then scaled according to input rate)
// told_next_spk specifies the real time of spike (warped)
// So, the length of a warped simulation time slot is step and
// the length of a unwarped simulation time slot is step*mean_firing_rate, this is step/inp_pix_per
vector<spike_t> SpikingOutput::stochastic_spike_generation(unsigned long out_neu_idx, double input_val, CImg<double>::iterator next_spk_time_it, CImg<double>::iterator last_spk_time_it, CImg<double>::iterator curr_ref_period_it){
    vector<spike_t> slot_spks; // Temporal vector of output spikes for current sim. time slot
    // Intermediate variables used to calculate next spike time
    double inp_pix_per;
    double tslot_start, slot_len;

    // All calculations are done in whole units, so convert class time properties (in ms) into seconds
    tslot_start = simTime / 1000.0; // Start time of the current sim. slot: Convert it into seconds
    slot_len = step / 1000.0; // Length in time of a simulation slot (step)

    inp_pix_per = inp_pixel_to_period(input_val); // Convert input pixel magnitude into firing period in seconds
    // If input is not zero, we do not have to calculate spike times but the expression for updating
    // the next spike time becomes an indeterminate form, so evaluate its limit
    if(isfinite(inp_pix_per)) {
        double new_spk_time; // New spike time relative to current time slot start
        spike_t new_spk;

        new_spk.neuron = out_neu_idx; // Index of output neuron are assigned in the same way as Cimg pixel offsets
        // We can have several spikes in a single simulation time slot: iterate
        while((new_spk_time = apply_ref_period(*next_spk_time_it * inp_pix_per, *last_spk_time_it - tslot_start, *curr_ref_period_it)) < slot_len){ // warp next spike time accoring to input rate to get the desired firing rate and apply ref. period
            double firing_period;

            new_spk.time = tslot_start + new_spk_time; // Time of next spike
            // These conditions should never be met:
            if(new_spk.time < tslot_start)
                cout << "Internal error: a spike for a previous simulation step has been generated. current step [" << tslot_start << "," << tslot_start + slot_len << ") spike time:" << new_spk.time << "s" << endl;
            if(!isfinite(*next_spk_time_it))
                cout << "Internal error: spike time could not be calculated (indeterminate form). current step [" << tslot_start << "," << tslot_start + slot_len << ") spike time:" << new_spk.time << endl;

            slot_spks.push_back(new_spk); // Insert spike in list
            *last_spk_time_it = new_spk.time; // Update last spike time
            renew_ref_period_val(curr_ref_period_it);
            
            // Determine firing period in the "unwarped" time slot
            if(isfinite(Spike_dist_shape)) // Select stochastic or deterministic spike times
                firing_period = gam_dist(rand_gen);
            else // Spike_dist_shape is infinite (not specified), so we do not use stochasticity
                firing_period = 1; // 1Hz is the firing freq. in a "unwarped" time slot
        
            *next_spk_time_it = *next_spk_time_it + firing_period; // Update the time of predicted next firing for this neuron
        }
        *next_spk_time_it = *next_spk_time_it - slot_len/inp_pix_per; // Make *next_spk_time_it relative to the next time slot
    }
    return(slot_spks);
}


//------------------------------------------------------------------------------//

// function used to compare spikes according to time (and neuron index) when
// sorting in SpikingOutput::update()
bool spk_time_comp(spike_t spk1, spike_t spk2){
    bool comp_result;
    double time_diff;
    
    time_diff = spk2.time - spk1.time;
    if(time_diff > 0)
        comp_result=true; // spk1.time < spk2.time
    else if(time_diff < 0)
        comp_result=false; // spk1.time > spk2.timeinp_pix_per
    else // This subordering it is implemented just to ease the visual inspection of the output file
        comp_result = spk1.neuron < spk2.neuron;
    
    return(comp_result);
    }

void SpikingOutput::update(){
    unsigned long out_neu_idx; // Index to the current neuron (or image pixel)
    CImg<double>::iterator inp_img_it = inputImage->begin();
    CImg<double>::iterator next_spk_time_it = next_spk_time->begin();
    CImg<double>::iterator last_spk_time_it = last_spk_time->begin();
    CImg<double>::iterator curr_ref_period_it = curr_ref_period->begin();
    vector<spike_t> slot_spks; // Temporal vector of output spikes for current sim. time slot

    out_neu_idx=0UL;
    inp_img_it+=First_inp_ind; // start from the pixl selected by user
    next_spk_time_it+=First_inp_ind;
    last_spk_time_it+=First_inp_ind;
    curr_ref_period_it+=First_inp_ind;
    // For each image input pixel:
    while(inp_img_it<inputImage->end() && out_neu_idx<Total_inputs){ // we use inp_img_it.end() as upper bound for all iterators
        vector<spike_t> neu_spks;
        double input_val=*inp_img_it;
        
        neu_spks = stochastic_spike_generation(out_neu_idx, input_val, next_spk_time_it, last_spk_time_it, curr_ref_period_it);
        
        slot_spks.insert(slot_spks.end(), neu_spks.begin(), neu_spks.end()); // Append spikes of current neuron
        
        // Switch to the next neuron (pixel)
        inp_img_it+=Inp_ind_inc;
        next_spk_time_it+=Inp_ind_inc;
        last_spk_time_it+=Inp_ind_inc;
        curr_ref_period_it+=Inp_ind_inc;
        out_neu_idx++;
    }

    // Some programs may require that the spikes are issued in time order
    // So, sort the spikes of current sim. slot before inserting them in the class
    // output spike list
    std::sort(slot_spks.begin(), slot_spks.end(), spk_time_comp);
    out_spks.insert(out_spks.end(), slot_spks.begin(), slot_spks.end());
}

//------------------------------------------------------------------------------//

// Implement a method to write a spike_t object to a ouput stream
// This method is used in SpikingOutput::SaveFile()
ostream& operator<< (ostream& out, const spike_t& spk) {
    out << setprecision(9) << spk.neuron << " " << spk.time;
    return out;
}

// This function is executed when the class object is destructed to save
// the output activity (spikes) generated during the whole simulation
bool SpikingOutput::SaveFile(string spk_filename){
    bool ret_correct;
    
    ofstream out_spk_file(spk_filename, ios::out);
    ret_correct=out_spk_file.is_open();
    if(ret_correct){
        out_spk_file << "% Output activity file generated by COREM";
        // get current local time and log it on the output file
        time_t time_as_secs = time(NULL);
        if(time_as_secs != (time_t)-1){
            struct tm *time_local = localtime(&time_as_secs);
            if(time_local != NULL){
                out_spk_file << " on " << asctime(time_local); // This string includes \n
            }
            else
                out_spk_file << endl;
        }
        else
            out_spk_file << endl;
        out_spk_file << "% <neuron index from 0> <spike time in seconds>" << endl;
        ostream_iterator<spike_t> out_spk_it(out_spk_file, "\n"); // Add \n at the end of each line
        copy(out_spks.begin(), out_spks.end(), out_spk_it); // Write all spikes to file
    
        out_spk_file.close();
    }
    else
        cout << "Unable to open file for output spikes: " << spk_filename << endl;
  
    return(ret_correct);
}

//------------------------------------------------------------------------------//

// This function is normally neither needed nor used
CImg<double>* SpikingOutput::getOutput(){
    return inputImage;
}

//------------------------------------------------------------------------------//

bool SpikingOutput::isDummy() {
    return false;
    };