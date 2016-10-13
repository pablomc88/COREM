#include <algorithm> // for std::sort
#include <iostream>

// For file writing:
#include <fstream>
#include <iterator>
#include <string>

#include <random> // To generate neuron noise
#include <ctime> // To log the current time in output spike file
#include <limits>

#include "RetinaOutput.h"

RetinaOutput::RetinaOutput(int x, int y, double temporal_step):module(x,y,temporal_step){

    // conversion parameters default value
    Max_freq=1000;
    Min_freq=0;
    Input_threshold=0;
    Spk_freq_per_inp=1;
    out_spk_filename="spikes.spk";
    Noise_std_dev=0.4;
    
    // Input buffer
    inputImage=new CImg<double> (sizeY, sizeX, 1, 1, 0);

    // Internal state variables: initial value
    last_spk_time=new CImg<double> (sizeY, sizeX, 1, 1, 0);
    next_spk_time=new CImg<double> (sizeY, sizeX, 1, 1, numeric_limits<double>::infinity()); // For a 0 input the next spike time is infinity
}

RetinaOutput::RetinaOutput(const RetinaOutput &copy):module(copy){

    //step=copy.step;
    //sizeX=copy.sizeX;
    //sizeY=copy.sizeY;

    Max_freq = copy.Max_freq;
    Min_freq = copy.Min_freq;
    Input_threshold = copy.Input_threshold;
    Spk_freq_per_inp = copy.Spk_freq_per_inp;

    inputImage=new CImg<double> (sizeY,sizeX,1,1,0.0);
    last_spk_time=new CImg<double> (sizeY,sizeX,1,1,0.0);
    next_spk_time=new CImg<double> (sizeY,sizeX,1,1,0.0);
}

RetinaOutput::~RetinaOutput(){

    // Save generated spikes before destructing the object
    cout << "Saving output spike file: " << out_spk_filename << "... " << flush;
    cout << (SaveFile(out_spk_filename)?"Ok":"Fail") << endl;
    
    if(inputImage) delete inputImage;
    if(last_spk_time) delete last_spk_time;

    if(next_spk_time) delete next_spk_time;
}

//------------------------------------------------------------------------------//

void RetinaOutput::allocateValues(){
    module::allocateValues(); // Use the allocateValues() method of the base class
}

RetinaOutput& RetinaOutput::set_Max_freq(double max_spk_freq){
    if (max_spk_freq>=0)
        Max_freq = max_spk_freq;
    return(*this); // Return value is really not used
}

RetinaOutput& RetinaOutput::set_Min_freq(double min_spk_freq){
    if (min_spk_freq>=0)
        Min_freq = min_spk_freq;
    return(*this);
}

RetinaOutput& RetinaOutput::set_Input_threshold(double input_threshold){
    if (input_threshold>=0)
        Input_threshold = input_threshold;
    return(*this);
}

RetinaOutput& RetinaOutput::set_Freq_per_inp(double freq_per_inp_unit){
    if (freq_per_inp_unit>=0)
        Spk_freq_per_inp = freq_per_inp_unit;
    return(*this);
}

RetinaOutput& RetinaOutput::set_Out_filename(string filename){
    out_spk_filename = filename;
    return(*this);
}

RetinaOutput& RetinaOutput::set_Noise_std_dev(double sigma_val){
    if (sigma_val>=0)
        Noise_std_dev = sigma_val;
    return(*this);
}
//------------------------------------------------------------------------------//

bool RetinaOutput::setParameters(vector<double> params, vector<string> paramID){

    bool correct = true;

    for (vector<double>::size_type i = 0;i<params.size();i++){
        const char * s = paramID[i].c_str();

        if (strcmp(s,"Max_freq")==0){
            set_Max_freq(params[i]);
        }else if (strcmp(s,"Min_freq")==0){
            set_Min_freq(params[i]);
        }
        else if (strcmp(s,"Input_threshold")==0){
            set_Input_threshold(params[i]);
        }
        else if (strcmp(s,"Freq_per_inp")==0){
            set_Freq_per_inp(params[i]);
        }
        else if (strcmp(s,"Noise_std_dev")==0){
            set_Noise_std_dev(params[i]);
        }
        else{
              correct = false;
        }
    }

    return correct;
}

//------------------------------------------------------------------------------//

void RetinaOutput::feedInput(double sim_time, const CImg<double>& new_input,bool isCurrent,int port){
    // Ignore port type and copy input image
    *inputImage = new_input;
    // Update the current simulation time
    simTime = sim_time;
}

double RetinaOutput::inp_pixel_to_freq(double pixel_value){
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
// sorting in RetinaOutput::update()
bool spk_time_comp(spike_t spk1, spike_t spk2){
    bool comp_result;
    double time_diff;
    
    time_diff = spk2.time - spk1.time;
    if(time_diff > 0)
        comp_result=true; // spk1.time < spk2.time
    else if(time_diff < 0)
        comp_result=false; // spk1.time > spk2.time
    else // This subordering it is implemented just to ease the visual inspection of the output file
        comp_result = spk1.neuron < spk2.neuron;
    
    return(comp_result);
    }

void RetinaOutput::update(){

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
        double tnewf1, toldf0, toldf1;
        double tslotst, slot_len;
        double sqrt_eq;

        // All calculations are done in whole units, so convert class time properties (in ms) into seconds
        tslotst = simTime / 1000.0; // Start time of the current sim. slot: Convert it into seconds
        slot_len = step / 1000.0; // Length in time of a simulation slot (step)

        inp_pix_freq = inp_pixel_to_freq(*inp_img_it); // Convert input pixel magnitude into firing rate
        if(inp_pix_freq > 0) { // If input is not zero, we have to calculate spike times
            inp_pix_per = 1 / inp_pix_freq; // Convert firing rate (Hz) into firing period

            // Time of the first spike that would be generated in current update() call if the
            // previous pixel magnitude were the same as the current one:
            tnewf1 = *last_spk_time_it + inp_pix_per;
            // Time of the last spike that was generated (in a previous sim. slot):
            toldf0 = *last_spk_time_it;
            // Time of the first spike that would be generated if the current pixel magnitude and
            // next ones were the same as the previous ones:
            toldf1 = *next_spk_time_it;
            if(toldf1 > tslotst+inp_pix_per) // Limit the diminishing effect of the previous activity
                toldf1 = tslotst+inp_pix_per; // Same effect as no previous activity
            
            // We assume that the pixel magnitude changes when the simulation time slot changes, 
            // that is at tslotst.
            // So, depending on the relative position of the first spike of the new slot
            // and the relative position of the last (non-emitted) spike prediction of prevous slot
            // in realation to the slot start, we predict an intermediate first spike time
            // of the new slot (*next_spk_time_it). For that, we solve the next quadratic equation:
            // *next_spk_time_it =      (tslotst-toldf0)/(*next_spk_time_it-toldf0)*toldf1 +
            //                     (1 - (tslotst-toldf0)/(*next_spk_time_it-toldf0))*tnewf1
            // This equation defines *next_spk_time_it as weighted average of toldf1 and tnewf1,
            // so, *next_spk_time_it is between these two time values
            sqrt_eq=tnewf1*tnewf1 + toldf0*toldf0 + 4*toldf1*tslotst \
                    -4*toldf0*toldf1 - 4*tnewf1*tslotst + 2*tnewf1*toldf0;

            if(sqrt_eq < 0){ // Floatring point precission errors may lead to small negative values
                // cout << "precission error in spk predict eq.: sqrt(" << sqrt_eq << ")" << endl;
                sqrt_eq=0.0; // We cannot calculate the square root of a negative value, even if it is very small
            }
            *next_spk_time_it = (tnewf1 + toldf0 + sqrt(sqrt_eq)) / 2; // Solve second order equation

            // Another simpler implementation would be just calculate the next spike time from the last
            // emitted smike: *next_spk_time_it = tnewf1; ignoring the exact occurrence of the slot time

            new_spk.neuron=out_neu_idx; // Index of output neuron are assigned in the same way as Cimg pixel offsets

            if(*next_spk_time_it >= tslotst) // This should always be true for the equation solution 
                new_spk.time = *next_spk_time_it;
            else
                new_spk.time = tslotst; // We should not generate spikes for the previous sim. slot

            // We can have several spike in a single simulation time slot
            for(;new_spk.time < tslotst + slot_len; new_spk.time += inp_pix_per){ // We can have several spikes per sim. slot
                slot_spks.push_back(new_spk);
                *last_spk_time_it = new_spk.time; // Update the time of last firing for this neuron
                *next_spk_time_it = new_spk.time + inp_pix_per; // Update the time of predicted next firing for this neuron
            }
        }
        else // Input is zero, next spike will be emitted at infinity time
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
// This method is used in RetinaOutput::SaveFile()
ostream& operator<< (ostream& out, const spike_t& spk) {
    out << spk.neuron << " " << spk.time;
    return out;
}

// This function is neither needed nor used
bool RetinaOutput::SaveFile(string spk_filename){
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
CImg<double>* RetinaOutput::getOutput(){
    return inputImage;
}
