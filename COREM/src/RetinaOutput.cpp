#include <algorithm> // for std::sort
#include "RetinaOutput.h"


RetinaOutput::RetinaOutput(int x, int y, double temporal_step):module(x,y,temporal_step){

    // conversion parameters default value
    Max_freq=100;
    Min_freq=5;
    Input_threshold=0;
    Spks_per_inp=5;
    
    // Input buffer
    inputImage=new CImg<double> (sizeY,sizeX,1,1,0.0);

    // Internal state variables
    last_spk_time=new CImg<double> (sizeY,sizeX,1,1,0.0);
    next_spk_time=new CImg<double> (sizeY,sizeX,1,1,0.0);
}

RetinaOutput::RetinaOutput(const RetinaOutput &copy):module(copy){

    //step=copy.step;
    //sizeX=copy.sizeX;
    //sizeY=copy.sizeY;

    Max_freq = copy.Max_freq;
    Min_freq = copy.Min_freq;
    Input_threshold = copy.Input_threshold;
    Spks_per_inp = copy.Spks_per_inp;

    inputImage=new CImg<double> (sizeY,sizeX,1,1,0.0);
    last_spk_time=new CImg<double> (sizeY,sizeX,1,1,0.0);
    next_spk_time=new CImg<double> (sizeY,sizeX,1,1,0.0);
}

RetinaOutput::~RetinaOutput(){

    if(inputImage) delete inputImage;
    if(last_spk_time) delete last_spk_time;

    if(next_spk_time) delete next_spk_time;
}

//------------------------------------------------------------------------------//

void RetinaOutput::allocateValues(){
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

RetinaOutput& RetinaOutput::set_Spks_per_inp(double freq_per_inp_unit){
    if (freq_per_inp_unit>=0)
        Spks_per_inp = freq_per_inp_unit;
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
        else if (strcmp(s,"Spks_per_inp")==0){
            set_Spks_per_inp(params[i]);
        }
        else{
              correct = false;
        }
    }

    return correct;
}

//------------------------------------------------------------------------------//

void RetinaOutput::feedInput(const CImg<double>& new_input,bool isCurrent,int port){
    // Ignore port type and copy input image
    *inputImage = new_input;
}

double RetinaOutput::inp_pixel_to_freq(double pixel_value){
    double firing_rate;
    
    if(pixel_value < Input_threshold) // If value under threshold, no output activity must be generated
        firing_rate=0;
    else
        firing_rate=(pixel_value-Input_threshold)*Spks_per_inp + Min_freq;
    
    if(firing_rate > Max_freq) // Firing rate is saturated
       firing_rate=Max_freq;

    return(firing_rate);
}
//------------------------------------------------------------------------------//
// function used to compare spikes according to time when sorting
bool spk_time_comp(spike_t spk1, spike_t spk2){
    return (spk1.time<spk2.time);
    }

void RetinaOutput::update(){

    unsigned long out_neu_idx; // Index to the current neuron (or image pixel)
    CImg<double>::iterator inp_img_it = inputImage->begin();
    CImg<double>::iterator last_spk_time_it = last_spk_time->begin();
    CImg<double>::iterator next_spk_time_it = next_spk_time->begin();
    vector<spike_t> slot_spks; // Temporal vector of output spikes for current sim. time slot

    out_neu_idx=0UL;
    // For each input image pixel
    while(inp_img_it<inputImage->end()){  // we use inp_img_it.end() as upper bound for all iterators
        spike_t new_spk;
        double inp_pix_per;
        double tnewf1, toldf0, toldf1;
        double tslotst;
        inp_pix_per = 1 / inp_pixel_to_freq(*inp_img_it); // Convert input pixel magnitude into firing period
        
        // Time of the first spike that would be generated in current update() call if we the
        // previous pixel magnitude were the same as the current one
        tnewf1 = *last_spk_time_it + inp_pix_per;
        // Time of the last spike that was generated (in the previous sim. slot)
        toldf0 = *last_spk_time_it;
        // Time of the first spike that would be generated if the current pixel magnitude were
        // the same as the previous one
        toldf1 = *next_spk_time_it;
        // Start time of the current sim. slot
        tslotst = step * 1; // <-------- correct it
        
        // We assume that the pixel magnitude changes when the simulation slot changes,
        // Depending on the relative position of the last spike of the prevous slot and
        // the first spikes of the new slot to the slot start, we calculate the
        // first spike time of the new slot.
        new_spk.time = (tnewf1 + toldf0 + sqrt(tnewf1*tnewf1 + toldf0*toldf0 + 4*toldf1*tslotst \
                                               -4*toldf0*toldf1 - 4*tnewf1*tslotst + 2*tnewf1*toldf0)) / 2;
        new_spk.neuron=out_neu_idx; // Index of output neuron are assigned in the same way as Cimg pixels

        if(new_spk.time < tslotst) // We should not generate spikes for the previous sim. slot
            new_spk.time = tslotst;
        
        for(;new_spk.time < tslotst + step; new_spk.time += inp_pix_per){ // We can have several spikes per sim. slot
            slot_spks.push_back(new_spk);
            *last_spk_time_it = new_spk.time; // Update the time of last firing for this neuron
            *next_spk_time_it = new_spk.time + inp_pix_per; // Update the time of predicted next firing for this neuron
        }
        
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

// This function is neither needed nor used
CImg<double>* RetinaOutput::getOutput(){
    return inputImage;
}
