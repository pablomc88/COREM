#include <algorithm> // for std::sort
#include <iostream>

// For file writing:
#include <fstream>
#include <iterator>
#include <string>

#include <random> // To generate neuron noise
#include <ctime> // To log the current time in output spike file
#include <limits>

#include "SpikingOutput.h"

SpikingOutput::SpikingOutput(int x, int y, double temporal_step, string output_filename):module(x,y,temporal_step){
    
    // input-to-spike-time conversion parameters (default values)
    Max_period=numeric_limits<double>::infinity();
    Min_period=0.0;
    Input_threshold=0;
    Spk_freq_per_inp=1;
    Spike_std_dev=0.0;
    Limit_std_dev=0.0;
    if(output_filename.compare("") != 0) 
        out_spk_filename=output_filename;
    else
        out_spk_filename="results/spikes.spk";
    // Save all input images by default
    Start_time=0.0;
    End_time=numeric_limits<double>::infinity();
    Random_init=false;
    
    normal_distribution<double>::param_type init_norm_dist_params(0.0, Limit_std_dev); // mean=0, sigma=Noise_std_dev
    norm_dist.param(init_norm_dist_params); // Set initial params of normal distribution
    
    uniform_real_distribution<double>::param_type init_unif_dist_params(-1.0, 0); // Numbers from -1 to 0
    unif_dist.param(init_unif_dist_params); // Set initial params of uniform distribution
    
    // gamma_distribution<double>::param_type init_gam_dist_params(1.0, 1.0); // (these particular init values are useless) first param=k, second param=theta
    // gam_dist.param(init_gam_dist_params); // Set initial params of gamma distribution
    
    // Input buffer
    inputImage=new CImg<double> (sizeY, sizeX, 1, 1, 0);

    // Internal state variables: initial value
    last_firing_period=new CImg<double> (sizeY, sizeX, 1, 1, numeric_limits<double>::infinity()); // Previous input = 0 -> perdiod=inifinity
    next_spk_time=new CImg<double> (sizeY, sizeX, 1, 1, 0.0); // Next predicted spike time = 0s
}

SpikingOutput::SpikingOutput(const SpikingOutput &copy):module(copy){

    Max_period = copy.Max_period;
    Min_period = copy.Min_period;
    Input_threshold = copy.Input_threshold;
    Spk_freq_per_inp = copy.Spk_freq_per_inp;
    Spike_std_dev = copy.Spike_std_dev;
    Limit_std_dev = copy.Limit_std_dev;
    out_spk_filename = copy.out_spk_filename;
    Random_init = copy.Random_init;
    norm_dist = copy.norm_dist;
    unif_dist = copy.unif_dist;

    inputImage=new CImg<double>(*copy.inputImage);
    last_firing_period=new CImg<double>(*copy.last_firing_period);
    next_spk_time=new CImg<double>(*copy.next_spk_time);
}

SpikingOutput::~SpikingOutput(){

    // Save generated spikes before destructing the object
    cout << "Saving output spike file: " << out_spk_filename << "... " << flush;
    cout << (SaveFile(out_spk_filename)?"Ok":"Fail") << endl;
    
    delete inputImage;
    delete last_firing_period;
    delete next_spk_time;
}

//------------------------------------------------------------------------------//

void SpikingOutput::randomize_state(){
    double init_retentivity = 1;
    CImg<double>::iterator last_firing_period_it = last_firing_period->begin();
    CImg<double>::iterator next_spk_time_it = next_spk_time->begin();

    while(last_firing_period_it < last_firing_period->end()){ // For every spiking output
        // To randomize the state of each output, we set the last firing period
        // to 1 (that is, next_spk_time-last_spk_time = 1), and we choose
        // randomly the time at which each output fired last between -1 and 0.
        // In this way (told_spk1-tslot_start) / (told_spk1-told_spk0) is always
        // a number between 0 and 1, and the first firing phase is random.
        *last_firing_period_it = unif_dist(rand_gen)*init_retentivity; // Uniform number between -1 and 0 seconds
        *next_spk_time_it = *last_firing_period_it+init_retentivity;
        last_firing_period_it++;
        next_spk_time_it++;
    }
}

//------------------------------------------------------------------------------//

void SpikingOutput::allocateValues(){
    module::allocateValues(); // Use the allocateValues() method of the base class

    // Set parameters of distributions for random number generation
    normal_distribution<double>::param_type init_params(0.0, Limit_std_dev); // (mean=0, sigma=Limit_std_dev)
    norm_dist.param(init_params); // Set initial params of normal distribution
    
    // Resize initial value
    inputImage->assign(sizeY, sizeX, 1, 1, 0);
    last_firing_period->assign(sizeY, sizeX, 1, 1, numeric_limits<double>::infinity()); // For a 0 input the firing period is infinity
    next_spk_time->assign(sizeY, sizeX, 1, 1, 0.0);

    if(Random_init) // If parameter Random_init is set to true, init the state of outputs randomly
        randomize_state();
}

SpikingOutput& SpikingOutput::set_Max_period(double max_spk_per){
    if (max_spk_per>=0)
        Max_period = max_spk_per;
    return(*this); // Return value is really not used but we return it to avoid warnings
}

SpikingOutput& SpikingOutput::set_Min_period(double min_spk_per){
    if (min_spk_per>=0)
        Min_period = min_spk_per;
    return(*this);
}

SpikingOutput& SpikingOutput::set_Input_threshold(double input_threshold){
    if (input_threshold>=0)
        Input_threshold = input_threshold;
    return(*this);
}

SpikingOutput& SpikingOutput::set_Freq_per_inp(double freq_per_inp_unit){
    if (freq_per_inp_unit>=0)
        Spk_freq_per_inp = freq_per_inp_unit;
    return(*this);
}

SpikingOutput& SpikingOutput::set_Spike_std_dev(double std_dev_val){
    if (std_dev_val>=0)
        Spike_std_dev = std_dev_val;
    return(*this);
}

SpikingOutput& SpikingOutput::set_Limit_std_dev(double std_dev_val){
    if (std_dev_val>=0)
        Limit_std_dev = std_dev_val;
    return(*this);
}

SpikingOutput& SpikingOutput::set_Start_time(double start_time){
    if (start_time>=0)
        Start_time = start_time;
    return(*this);
}

SpikingOutput& SpikingOutput::set_End_time(double end_time){
    if (end_time>=0)
        End_time = end_time;
    return(*this);
}

SpikingOutput& SpikingOutput::set_Random_init(bool rnd_init){
    Random_init = rnd_init;
    return(*this);
}

//------------------------------------------------------------------------------//

bool SpikingOutput::setParameters(vector<double> params, vector<string> paramID){

    bool correct = true;

    for (vector<double>::size_type i = 0;i<params.size();i++){
        const char * s = paramID[i].c_str();

        if (strcmp(s,"Max_period")==0){
            set_Max_period(params[i]);
        } else if (strcmp(s,"Min_period")==0){
            set_Min_period(params[i]);
        } else if (strcmp(s,"Input_threshold")==0){
            set_Input_threshold(params[i]);
        } else if (strcmp(s,"Freq_per_inp")==0){
            set_Freq_per_inp(params[i]);
        } else if (strcmp(s,"Spike_std_dev")==0){
            set_Spike_std_dev(params[i]);
        } else if (strcmp(s,"Limit_std_dev")==0){
            set_Limit_std_dev(params[i]);
        } else if (strcmp(s,"Start_time")==0){
            set_Start_time(params[i]);
        } else if (strcmp(s,"End_time")==0){
            set_End_time(params[i]);
        } else if (strcmp(s,"Random_init")==0){
            set_Random_init(params[i] > 0.0);
        } else {
            correct = false;
        }
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

double SpikingOutput::inp_pixel_to_period(double pixel_value){
    double firing_per;
    double cur_min_per, cur_max_per;
    
    if(Limit_std_dev > 0){ // Soft limits chosen
        cur_min_per=Min_period + norm_dist(rand_gen); // / sqrt(step/1000.0); //  Add Gaussian white noise. We divide by sqrt(step/1000) to make the noise independent of the time step length (As in DOI:10.1523/JNEUROSCI.3305-05.2005)
        cur_max_per=Max_period + norm_dist(rand_gen);        
    } else { // No noise in freq limit: use hard limits
        cur_min_per=Min_period;
        cur_max_per=Max_period;
    }
    
        
    if(pixel_value < Input_threshold || Spk_freq_per_inp == 0.0) // If value under threshold, output activity has about 0Hz
        firing_per=numeric_limits<double>::infinity();
    else
        firing_per=1/(pixel_value-Input_threshold)*Spk_freq_per_inp + Min_period;
        
    // Draw the firing period from a gamma distribution as observed in rat ganglion cells: doi:10.1017/S095252380808067X
    if(Spike_std_dev > 0.0 && firing_per < numeric_limits<double>::infinity()){
        double gam_k, gam_theta;
        // Set params of gamma distribution according to current firing period and specified std dev
        // mean firing rate=k*tetha variance=k*theta^2
        // Solving the above eq. for k and theta we get:
        gam_k = firing_per*firing_per/(Spike_std_dev*Spike_std_dev);
        gam_theta = Spike_std_dev*Spike_std_dev/firing_per;
        gamma_distribution<double>::param_type init_gam_dist_params(gam_k, gam_theta);
        gam_dist.param(init_gam_dist_params);
        
        firing_per = gam_dist(rand_gen) - 0.01;
    }

    // Firing rate is saturated
    if(firing_per < cur_min_per)
       firing_per=cur_min_per;
    if(firing_per > cur_max_per)
       firing_per=cur_max_per;

    return(firing_per);
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
    CImg<double>::iterator last_firing_period_it = last_firing_period->begin();
    CImg<double>::iterator next_spk_time_it = next_spk_time->begin();
    vector<spike_t> slot_spks; // Temporal vector of output spikes for current sim. time slot

    out_neu_idx=0UL;
    // For each input image pixel:
    while(inp_img_it<inputImage->end()){ // we use inp_img_it.end() as upper bound for all iterators
        spike_t new_spk;
        // Intermediate variables used to calculate next spike time
        double inp_pix_per;
        double old_per, told_next_spk;
        double tslot_start, slot_len;

        // All calculations are done in whole units, so convert class time properties (in ms) into seconds
        tslot_start = simTime / 1000.0; // Start time of the current sim. slot: Convert it into seconds
        slot_len = step / 1000.0; // Length in time of a simulation slot (step)

        inp_pix_per = inp_pixel_to_period(*inp_img_it); // Convert input pixel magnitude into firing period in seconds
        // If input is not zero, we do not have to calculate spike times but the expression for updating
        // the next spike time becomes an indeterminate form, so evaluate its limit
        if(inp_pix_per < numeric_limits<double>::infinity()) {

            // Firing period of the last non-zero input (in a previous sim. slot):
            old_per = *last_firing_period_it;
            // Time of the first spike that would be now generated if the current pixel magnitude were
            // the same as the previous one:
            told_next_spk = *next_spk_time_it;
            
            // To calculate the next spike time we consider how far the current slot start is from the next
            // predicted firing time for previous input (told_next_spk). If they are almost conincident,
            // the neuron was expected to fire at that time, so it can fire at the start of the slot.
            // If they are very far (in relation to the firing period), the neuron has just fired, then we delay
            // the firing up to the firing period for the current input (inp_pix_per).
            // This algorithm preverves the firing rate among slot if the input is constant
            // The value of the fraction (told_next_spk-tslot_start)/told_next_spk should be between 0 and 1,
            // so the next spike should be emitted between the slot start and the slot start plus the current firing period (inp_pix_per)
            *next_spk_time_it = tslot_start + (told_next_spk-tslot_start) * inp_pix_per / old_per;
            *last_firing_period_it = inp_pix_per; // Update next_spk_time and last_firing_period (in case do not fire in this time step but in a following one)
cout << "Pix: " << inp_img_it - inputImage->begin() << ": tn=" << *next_spk_time_it << " (t0=" << told_next_spk << " ts="<< tslot_start << " Tn=" << inp_pix_per << " To=" << old_per << ")" <<endl;
            new_spk.neuron = out_neu_idx; // Index of output neuron are assigned in the same way as Cimg pixel offsets
            new_spk.time = *next_spk_time_it;

if(new_spk.time < tslot_start) cout << "Internal error: a spike for a previous simulation step has been generated. step [" << tslot_start << "," << tslot_start + slot_len << ") spk:" << new_spk.time << endl;

            // We can have several spikes in a single simulation time slot
            for(;new_spk.time < tslot_start + slot_len;){
                slot_spks.push_back(new_spk);
                
                inp_pix_per = inp_pixel_to_period(*inp_img_it); // change value of inp_pix_per: this is usefull only if noise is activated
                if(isfinite(inp_pix_per)){
                    new_spk.time += inp_pix_per;
                    *next_spk_time_it = new_spk.time; // Update the time of predicted next firing for this neuron                
                    *last_firing_period_it = inp_pix_per; // Update the time of last firing for this neuron
                    cout << "pix per " << inp_pix_per << ". step [" << tslot_start << "," << tslot_start + slot_len << ") spk:" << new_spk.time << endl;
                } else {
                    *next_spk_time_it = tslot_start + slot_len;
                    // update *last_firing_period_it ?
                    cout << "ocurrio pix per " << inp_pix_per << ". step [" << tslot_start << "," << tslot_start + slot_len << ") spk:" << new_spk.time << endl;
                    break; // period=inf -> spike at infinite time: exit loop
                }
            }
        }
        else // Input is zero: prediction is delayed one simulation time step, so a new prediction is correctly calculated the next time step
            *next_spk_time_it = *next_spk_time_it + slot_len;
            
        // Switch to the next neuron (pixel)
        inp_img_it++;
        last_firing_period_it++;
        next_spk_time_it++;
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
    out << spk.neuron << " " << spk.time;
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
        ostream_iterator<spike_t> out_spk_it(out_spk_file, "\n");
        copy(out_spks.begin(), out_spks.end(), out_spk_it);
    
        out_spk_file.close();
    }
    else
        cout << "Unable to open file for output spikes: " << spk_filename << endl;
  
    return(ret_correct);
}

//------------------------------------------------------------------------------//

// This function is neither needed nor used
CImg<double>* SpikingOutput::getOutput(){
    return inputImage;
}

//------------------------------------------------------------------------------//

bool SpikingOutput::isDummy() {
    return false;
    };