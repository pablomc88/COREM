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
    last_refrac_end_time=new CImg<double> (sizeY,sizeX,1,1,0.0);
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
    last_refrac_end_time=new CImg<double> (sizeY,sizeX,1,1,0.0);
}

RetinaOutput::~RetinaOutput(){

    if(inputImage) delete inputImage;
    if(last_spk_time) delete last_spk_time;

    if(last_refrac_end_time) delete last_refrac_end_time;
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
    
    if(pixel_value<Input_threshold) // If value under threshold, no output activity must be generated
        firing_rate=0;
    else
        firing_rate=(pixel_value-pixel_value)*Spks_per_inp + Min_freq;
    
    if(firing_rate>Max_freq) // Firing rate is saturated
       Max_freq=0;

    return(firing_rate);
}

//------------------------------------------------------------------------------//

void RetinaOutput::update(){


}

//------------------------------------------------------------------------------//

// This function is neither needed nor used
CImg<double>* RetinaOutput::getOutput(){
    return inputImage;
}
