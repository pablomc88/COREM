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
    // conversion parameters default value
    Max_freq=1000;
    Min_freq=0;
    Input_threshold=0;
    Spk_freq_per_inp=1;
    Noise_std_dev=0.0;
    if(output_filename.compare("") != 0) 
        out_spk_filename=output_filename;
    else
        out_spk_filename="results/spikes.spk";
    // Save all input images by default
    Start_time=0.0;
    End_time=numeric_limits<double>::infinity();
    
    // Input buffer
    inputImage=new CImg<double> (sizeY, sizeX, 1, 1, 0);

    // Internal state variables: initial value
    last_spk_time=new CImg<double> (sizeY, sizeX, 1, 1, 0);
    next_spk_time=new CImg<double> (sizeY, sizeX, 1, 1, numeric_limits<double>::infinity()); // For a 0 input the next spike time is infinity
}

SpikingOutput::SpikingOutput(const SpikingOutput &copy):module(copy){

    Max_freq = copy.Max_freq;
    Min_freq = copy.Min_freq;
    Input_threshold = copy.Input_threshold;
    Spk_freq_per_inp = copy.Spk_freq_per_inp;
    Noise_std_dev = copy.Noise_std_dev;
    out_spk_filename = copy.out_spk_filename;

    inputImage=new CImg<double>(*copy.inputImage);
    last_spk_time=new CImg<double>(*copy.last_spk_time);
    next_spk_time=new CImg<double>(*copy.next_spk_time);
}

SpikingOutput::~SpikingOutput(){

    // Save generated spikes before destructing the object
    cout << "Saving output spike file: " << out_spk_filename << "... " << flush;
    cout << (SaveFile(out_spk_filename)?"Ok":"Fail") << endl;
    
    delete inputImage;
    delete last_spk_time;
    delete next_spk_time;
}

//------------------------------------------------------------------------------//

void SpikingOutput::allocateValues(){
    module::allocateValues(); // Use the allocateValues() method of the base class
    
    // Resize initial value
    inputImage->assign(sizeY, sizeX, 1, 1, 0);
    last_spk_time->assign(sizeY, sizeX, 1, 1, 0);
    next_spk_time->assign(sizeY, sizeX, 1, 1, numeric_limits<double>::infinity()); // For a 0 input the next spike time is infinity

}

SpikingOutput& SpikingOutput::set_Max_freq(double max_spk_freq){
    if (max_spk_freq>=0)
        Max_freq = max_spk_freq;
    return(*this); // Return value is really not used but we return it to avoid warnings
}

SpikingOutput& SpikingOutput::set_Min_freq(double min_spk_freq){
    if (min_spk_freq>=0)
        Min_freq = min_spk_freq;
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

SpikingOutput& SpikingOutput::set_Noise_std_dev(double sigma_val){
    if (sigma_val>=0)
        Noise_std_dev = sigma_val;
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

//------------------------------------------------------------------------------//

bool SpikingOutput::setParameters(vector<double> params, vector<string> paramID){

    bool correct = true;

    for (vector<double>::size_type i = 0;i<params.size();i++){
        const char * s = paramID[i].c_str();

        if (strcmp(s,"Max_freq")==0){
            set_Max_freq(params[i]);
        } else if (strcmp(s,"Min_freq")==0){
            set_Min_freq(params[i]);
        } else if (strcmp(s,"Input_threshold")==0){
            set_Input_threshold(params[i]);
        } else if (strcmp(s,"Freq_per_inp")==0){
            set_Freq_per_inp(params[i]);
        } else if (strcmp(s,"Noise_std_dev")==0){
            set_Noise_std_dev(params[i]);
        } else if (strcmp(s,"Start_time")==0){
            set_Start_time(params[i]);
        } else if (strcmp(s,"End_time")==0){
            set_End_time(params[i]);
        } else {
            correct = false;
        }
    }

    return correct;
}

//------------------------------------------------------------------------------//

void SpikingOutput::feedInput(double sim_time, const CImg<double>& new_input,bool isCurrent,int port){
    // Ignore port type and copy input image
    if(simTime >= Start_time && simTime+step <= End_time) // Check if the user wants to record the image at current time
        *inputImage = new_input;
    else
        inputImage->assign(); // Reset the input image to an empty image so that no spikes are generated during this sim. time step

    // Update the current simulation time
    simTime = sim_time;
}

double SpikingOutput::inp_pixel_to_freq(double pixel_value){
    double firing_rate;
    double freq_noise;
    
    if(Noise_std_dev > 0){
        normal_distribution<double> norm_dist(0.0, Noise_std_dev); // mean=0, sigma=Noise_std_dev
        freq_noise = norm_dist(rand_gen) / sqrt(step/1000.0); // As in DOI:10.1523/JNEUROSCI.3305-05.2005
    }
    else
        freq_noise=0;
        
    if(pixel_value < Input_threshold) // If value under threshold, no output activity must be generated
        firing_rate=0;
    else
        firing_rate=(pixel_value-Input_threshold)*Spk_freq_per_inp + Min_freq;
    
    firing_rate += freq_noise; // Add Gaussian white noise
    
    // Firing rate is saturated
    if(firing_rate > Max_freq)
       firing_rate=Max_freq;
    if(firing_rate < 0.0)
       firing_rate=0.0;

    return(firing_rate);
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
    CImg<double>::iterator last_spk_time_it = last_spk_time->begin();
    CImg<double>::iterator next_spk_time_it = next_spk_time->begin();
    vector<spike_t> slot_spks; // Temporal vector of output spikes for current sim. time slot

    out_neu_idx=0UL;
    // For each input image pixel:
    while(inp_img_it<inputImage->end()){ // we use inp_img_it.end() as upper bound for all iterators
        spike_t new_spk;
        // Intermediate variables used to calculate next spike time
        double inp_pix_freq, inp_pix_per;
        double told_spk0, told_spk1;
        double tslot_start, slot_len;

        // All calculations are done in whole units, so convert class time properties (in ms) into seconds
        tslot_start = simTime / 1000.0; // Start time of the current sim. slot: Convert it into seconds
        slot_len = step / 1000.0; // Length in time of a simulation slot (step)

        inp_pix_freq = inp_pixel_to_freq(*inp_img_it); // Convert input pixel magnitude into firing rate
        if(inp_pix_freq > 0) { // If input is not zero, we have to calculate spike times
            inp_pix_per = 1 / inp_pix_freq; // Convert firing rate (Hz) into firing period

            // Time of the last spike that was generated (in a previous sim. slot):
            told_spk0 = *last_spk_time_it;
            // Time of the first spike that would be now generated if the current pixel magnitude were
            // the same as the previous one:
            told_spk1 = *next_spk_time_it;
            
            // To calculate the next spike time we consider how far the current slot start is from the next
            // predicted firing time for previous input (told_spk1). If they are almost conincident,
            // The neuron was expected to fire at that time, so it can fire at the start of the slot.
            // If they are very far (in relation to the firing period), the neuron has just fired, then delay
            // the firing up to the firing period for the current input (inp_pix_per).
            // This algorithm preverves the firing rate among slot if the input is constant
            if(told_spk1 < numeric_limits<double>::infinity()) // Check that the previous input magnitude was different from zero
                // The value of the fraction should be between 0 and 1, so the next spike should be emitted
                // between the slot start and the slot start plus the current firing period (inp_pix_per)
                *next_spk_time_it = tslot_start + ((told_spk1-tslot_start) / (told_spk1-told_spk0)) * inp_pix_per;
            else // The input activity of previous slot was 0, so the previous expression is an indeterminate form, evaluate its limit
                *next_spk_time_it = tslot_start + inp_pix_per; // Continue to fire from the slot start plus current firing period
                // *next_spk_time_it = told_spk0 + inp_pix_per; // Or continue to fire from the last spike using current input (to avoid delays after 0 inputs)

            new_spk.neuron=out_neu_idx; // Index of output neuron are assigned in the same way as Cimg pixel offsets

            if(*next_spk_time_it >= tslot_start) // Ensure that we do not generate spikes for the previous sim. slot
                new_spk.time = *next_spk_time_it;
            else
                new_spk.time = tslot_start;

            // We can have several spikes in a single simulation time slot
            for(;new_spk.time < tslot_start + slot_len; new_spk.time += inp_pix_per){ // We can have several spikes per sim. slot
                slot_spks.push_back(new_spk);
                *last_spk_time_it = new_spk.time; // Update the time of last firing for this neuron
                *next_spk_time_it = new_spk.time + inp_pix_per; // Update the time of predicted next firing for this neuron
            }
        }
        else // Input is zero, next spike would be emitted at infinity time
            *next_spk_time_it = numeric_limits<double>::infinity();
            
        // Switch to the next neuron (pixel)
        inp_img_it++;
        last_spk_time_it++;
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