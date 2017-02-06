
#include <iostream>

// For file writing:
#include <fstream>
#include <string>
#include <stdio.h>
#include <string.h>

#include <limits>

#include "SequenceOutput.h"

SequenceOutput::SequenceOutput(int x, int y, double temporal_step, string output_filename):module(x,y,temporal_step){
    // Default output parameters
    Voxel_X_size=1.0;
    Voxel_Y_size=1.0;
    InFramesPerOut=1U;
    // Save all images by default
    Start_time=0.0;
    End_time=numeric_limits<double>::infinity();
    // Set output file name
    if(output_filename.compare("") != 0) 
        out_seq_filename=output_filename;
    else
        out_seq_filename="results/sequence.inr";
    
    // Input buffer
    inputImage=new CImg<double> (sizeY, sizeX, 1, 1, 0);

    // Internal state variables: number of frames already saved and skipped
    num_written_frames=0;
    num_skipped_frames=0;

    CreateINRFile(); // Create out_seq_filename file
}

// This method will probably not work properly because of the file sharing
SequenceOutput::SequenceOutput(const SequenceOutput &copy):module(copy){

    Voxel_X_size = copy.Voxel_X_size;
    Voxel_Y_size = copy.Voxel_Y_size;
    num_written_frames = copy.num_written_frames;
    num_skipped_frames = copy.num_skipped_frames;
    out_seq_filename = copy.out_seq_filename;
    Start_time = copy.Start_time;
    End_time = copy.End_time;
    InFramesPerOut = copy.InFramesPerOut;
    
    out_seq_file_handle.open(out_seq_filename, ios::out | ios::binary);
    if(out_seq_file_handle.is_open())
        out_seq_file_handle.seekp(INR_HEADER_LEN+sizeX*sizeY*sizeof(double)*num_written_frames, ios::beg); // move file pointer to the end of written data

    inputImage=new CImg<double>(*copy.inputImage);
}

SequenceOutput::~SequenceOutput(){

    // Save generated spikes before destructing the object
    cout << "Completing writing of output sequence file: " << out_seq_filename << "... " << flush;
    cout << (CloseINRFile()?"Ok":"Fail") << endl;
    
    delete inputImage;
}

//------------------------------------------------------------------------------//

bool SequenceOutput::allocateValues(){
    module::allocateValues(); // Use the allocateValues() method of the base class
    
    // Resize initial value
    inputImage->assign(sizeY, sizeX, 1, 1, 0);
    return(true);
}

bool SequenceOutput::set_Voxel_X_size(double voxel_x_size){
    bool ret_correct;
    if (voxel_x_size>=0) {
        Voxel_X_size = voxel_x_size;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SequenceOutput::set_Voxel_Y_size(double voxel_y_size){
    bool ret_correct;
    if (voxel_y_size>=0) {
        Voxel_Y_size = voxel_y_size;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SequenceOutput::set_Start_time(double start_time){
    bool ret_correct;
    if (start_time>=0) {
        Start_time = start_time;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SequenceOutput::set_End_time(double end_time){
    bool ret_correct;
    if (end_time>=0) {
        End_time = end_time;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SequenceOutput::set_InFramesPerOut(unsigned int n_frames){
    bool ret_correct;
    if (n_frames>0){
        InFramesPerOut = n_frames;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

//------------------------------------------------------------------------------//

bool SequenceOutput::setParameters(vector<double> params, vector<string> paramID){

    bool correct = true;

    for (vector<double>::size_type i = 0;i < params.size() && correct;i++){
        const char * s = paramID[i].c_str();

        if (strcmp(s,"Voxel_X_size")==0){
            correct = set_Voxel_X_size(params[i]);
        }else if (strcmp(s,"Voxel_Y_size")==0){
            correct = set_Voxel_Y_size(params[i]);
        }else if (strcmp(s,"Start_time")==0){
            correct = set_Start_time(params[i]);
        }else if (strcmp(s,"End_time")==0){
            correct = set_End_time(params[i]);
        }else if (strcmp(s,"InFramesPerOut")==0){
            correct = set_InFramesPerOut((unsigned int)(params[i]));
        } else{
              correct = false;
        }
    }
    return correct;
}

//------------------------------------------------------------------------------//

void SequenceOutput::feedInput(double sim_time, const CImg<double>& new_input,bool isCurrent,int port){
    // Ignore port type and copy input image
    *inputImage = new_input;
    // Update the current simulation time (although it is currently not used)
    simTime = sim_time;
}


//------------------------------------------------------------------------------//

void SequenceOutput::update(){
    if(simTime >= Start_time && simTime+step <= End_time) // Check if the user wants to record the image at current time
        if(++num_skipped_frames >= InFramesPerOut){ // Is time to write a new frame?
            WriteINRFrame(); // Save inputImage image into the file
            num_skipped_frames=0; // Reset the counter
        }
}

//------------------------------------------------------------------------------//


// This function is executed when the SequenceOutput object is created
bool SequenceOutput::CreateINRFile(){
    bool ret_correct;
    
    out_seq_file_handle.open(out_seq_filename, ios::out | ios::binary);

    ret_correct=out_seq_file_handle.is_open();
    if(ret_correct){
        out_seq_file_handle.seekp(INR_HEADER_LEN, ios::beg); // move put file pointer to leave space for the header beginning
        ret_correct = out_seq_file_handle.good();
    }
    else{
        cout << "Unable to create file for output sequence: " << out_seq_filename << endl;
        ret_correct = false;
    }
  
    return(ret_correct);
}

bool SequenceOutput::WriteINRFrame() {
    bool ret_correct;
    double *input_image_data;
    
    input_image_data = inputImage->data(); // Pointer to the first pixel value
    if(input_image_data != NULL){ // If the image is not empty
        out_seq_file_handle.write((char *)input_image_data, sizeX*sizeY*sizeof(double)); // Write last part of the header (which is fixed)
        ret_correct=out_seq_file_handle.good();
    } else
        ret_correct=true;
    if(ret_correct)
        num_written_frames++;
    return(ret_correct);
}

const char *SequenceOutput::getEndianness() {
    static const char *endianness_str[] = {"pc", "sun"};
    const int data = 1; // store an integer variable to check its encoding
    int endianness;
    
    endianness = ((char *)&data)[0] == 0; // 0=little-endian->Intel=pc 1=big-endian->SunSPARC=sun
    return(endianness_str[endianness]);
   }

bool SequenceOutput::CloseINRFile() {
    static const char INR_HEADER_START[]=\
         "#INRIMAGE-4#{\n"\
         "XDIM=%d\n"\
         "YDIM=%d\n"\
         "ZDIM=%d\n"\
         "VDIM=1\n"\
         "VX=%g\n"\
         "VY=%g\n"\
         "VZ=1\n"\
         "TYPE=double\n"\
         "PIXSIZE=%lu bits\n"\
         "SCALE=2**0\n"\
         "CPU=%s\n"; // INR header start
    static const char INR_HEADER_END[]="\n##}\n"; // chars at the End of INR header
    bool ret_correct;
    char inr_header[INR_HEADER_LEN];
    int n_printed_chars;
         
    snprintf(inr_header, INR_HEADER_LEN, INR_HEADER_START, sizeY, sizeX, num_written_frames, Voxel_X_size, Voxel_Y_size, sizeof(double)*8, getEndianness()); // popullate header buffer
    n_printed_chars = strlen(inr_header); // snprintf must always write a \0 char, so we can use strlen safely
    memset(inr_header+n_printed_chars, ' ', INR_HEADER_LEN-n_printed_chars); // Pad the remaining header buffer with spaces to fill the space which is not used
    memcpy(inr_header+INR_HEADER_LEN-(sizeof(INR_HEADER_END)-1), INR_HEADER_END, sizeof(INR_HEADER_END)-1); // Write the last part of the header

    out_seq_file_handle.seekp(0, ios::beg); // rewind put file pointer
    out_seq_file_handle.write(inr_header, INR_HEADER_LEN); // Write INR header

    ret_correct = out_seq_file_handle.good();
    out_seq_file_handle.close();
    
    return(ret_correct);
}

//------------------------------------------------------------------------------//

// This function is supposed not to be used
CImg<double>* SequenceOutput::getOutput(){
    return inputImage;
}

//------------------------------------------------------------------------------//

bool SequenceOutput::isDummy() {
    return false;
    };