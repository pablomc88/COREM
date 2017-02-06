
#include <iostream>
#include <algorithm> // std::sort
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include "SequenceInput.h"

SequenceInput::SequenceInput(int x, int y, double temporal_step, string input_file_path):module(x,y,temporal_step){
    // Default input parameters
    InputFilePath = input_file_path;
    SkipNInitFrames = 0; // No frame skipped by default
    RepeatLastFrame = false; // Sim. is terminted after end of input
    InputFramePeriod = 1; // by default one new frame is used each simulation millisecond
    CurrentInFrameInd = 0; // First frame to load is number 0
    verbose = true;
    // Allocate image buffers buffer
    outputImage = new CImg<double> (sizeY, sizeX, 1, 1, 0);

    // Init. internal vars
    NextFrameTime = 0; // First frame must be received at time 0
}

// This method will probably not be used
SequenceInput::SequenceInput(const SequenceInput &copy):module(copy){
    InputFilePath = copy.InputFilePath;
    SkipNInitFrames = copy.SkipNInitFrames;
    RepeatLastFrame = copy.RepeatLastFrame;
    InputFramePeriod = copy.InputFramePeriod;
    NextFrameTime = copy.NextFrameTime;
    CurrentInFrameInd = copy.CurrentInFrameInd;
    inputFileList = copy.inputFileList;
    inputMovie = copy.inputMovie;

    outputImage=new CImg<double>(*copy.outputImage);
}

SequenceInput::~SequenceInput(){
    closeInput();
    if(outputImage != NULL)
        delete outputImage;
}

//------------------------------------------------------------------------------//

bool SequenceInput::openInput(){
    bool ret_correct;
    struct stat input_seq_path_stat;

    ret_correct = false; // default return value

    // Check if the specified input-sequence path is a directory or a movie file
    if(stat(InputFilePath.c_str(), &input_seq_path_stat) == 0){
        if(verbose)
            cout << "Opening image sequence: " << InputFilePath << "..." << endl;
        if(S_ISDIR(input_seq_path_stat.st_mode)){ // The user has specified a directory as input sequence: load all the directory files
            DIR *dp; // Pointer to the opened input-directiry stream 
            dp = opendir(InputFilePath.c_str());
            if(dp != NULL){
                dirent* de;
                do{ // For each directory entry:
                    de = readdir(dp);
                    if(de != NULL){ // We got a valid entry
                        string curr_input_file_path(InputFilePath + de->d_name); // Compose the entire current dir entry path
                    
                        if(stat(curr_input_file_path.c_str(), &input_seq_path_stat) == 0){ // Try to get information about the de->d_name
                            if(!S_ISDIR(input_seq_path_stat.st_mode)){ // Subdirectories (including . and ..) are not included in the input file sequence
                                inputFileList.push_back(curr_input_file_path);
                                ret_correct = true; // At least one image was found, proceed
                            }
                        }else{
                            perror("Error accessing the specified input sequence directory: ");
                            break;
                        }
                    }
                }while (de != NULL); // Continue while we get valid directory entries
                closedir(dp);
                if(ret_correct){
                    sort(inputFileList.begin(), inputFileList.end()); // File will be load in alphabetical order
                    if(verbose)
                        cout << inputFileList.size() << " files in directory" << endl;
                }
            }else
                cout << "Error reading retina script: Cannot open input sequence directory " << InputFilePath << endl;

        } else { // The user has specified a file as input sequence: load the whole movie file
            inputMovie.load_inr(InputFilePath.c_str()); // We assume that the specified file is a movie (sequence of images)
            ret_correct = true;

            if(verbose)
                cout << inputMovie.depth() << " frames in movie file" << endl;
        }
    } else
        perror("Error accessing the specified input sequence: ");

    return(ret_correct);
}

void SequenceInput::closeInput(){
    // Nothing to do for the current implementation
}

void SequenceInput::skipFrame(){
    CurrentInFrameInd++;
}

//------------------------------------------------------------------------------//

bool SequenceInput::allocateValues(){
    bool ret_correct;
    module::allocateValues(); // Call the allocateValues() method of the base class
    
    ret_correct = openInput(); // Load INR file or open directory
    if(ret_correct){
        if(SkipNInitFrames>0)
            cout << "Skipping " << SkipNInitFrames << " input frames" << endl;
        for(int n_skipped_frames=0;n_skipped_frames<SkipNInitFrames;n_skipped_frames++)
            skipFrame(); // Skip frame
            
        // Use the first frame to find out the new dimensions of retina image size
        get_new_frame(); // Get first valid frame
        sizeY=outputImage->width();
        sizeX=outputImage->height();
        NextFrameTime=InputFramePeriod; // Next frame must be read at this time
        // output image should have been automatically resized after first frame load
    }

    return(ret_correct);
}

//------------------------------------------------------------------------------//

bool SequenceInput::set_SkipNInitFrames(int n_frames){
    bool ret_correct;
    if (n_frames>=0) {
        SkipNInitFrames = n_frames;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool SequenceInput::set_RepeatLastFrame(bool repeat_flag){
    RepeatLastFrame = repeat_flag;
    return(true);
}

bool SequenceInput::set_InputFramePeriod(double sim_time_period){
    bool ret_correct;
    if (sim_time_period>0) {
        InputFramePeriod = sim_time_period;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

//------------------------------------------------------------------------------//

bool SequenceInput::setParameters(vector<double> params, vector<string> paramID){

    bool correct = true;

    for (vector<double>::size_type i = 0;i < params.size() && correct;i++){
        const char * s = paramID[i].c_str();

        if (strcmp(s,"SkipNInitFrames")==0){
            correct = set_SkipNInitFrames((int)(params[i]));
        } else if (strcmp(s,"RepeatLastFrame")==0){
            correct = set_RepeatLastFrame(params[i] != 0.0);
        } else if (strcmp(s,"InputFramePeriod")==0){
            correct = set_InputFramePeriod(params[i]);
        } else{
              correct = false;
        }
    }
    return correct;
}

//------------------------------------------------------------------------------//

// This method can only be used to set the simulation time
void SequenceInput::feedInput(double sim_time, const CImg<double> &new_input, bool isCurrent, int port){
    // Update the current simulation time (although it is currently not used)
    simTime = sim_time;
}


//------------------------------------------------------------------------------//

void SequenceInput::get_new_frame(){    
    if(inputFileList.size() == 0){ // filename list is empty, so input was a movie file
        if(CurrentInFrameInd < (unsigned long)inputMovie.depth()) // Some frames still availables to be read
            *outputImage = inputMovie.get_slice(CurrentInFrameInd++);
        else
            if(outputImage != NULL && !RepeatLastFrame){
                if(verbose)
                    cout << "\rNo more input files: terminating simulation" << endl;
                delete outputImage;
                outputImage=NULL; // Indicate an end of input and simulation
            }
    
    } else { // Input was a directory
        if(CurrentInFrameInd < inputFileList.size()) // Some files still availables to be read
            outputImage->load(inputFileList.at(CurrentInFrameInd++).c_str());
        else {
            if(outputImage != NULL && !RepeatLastFrame){
                if(verbose)
                    cout << "\rNo more input frames: terminating simulation" << endl;
                delete outputImage;
                outputImage=NULL; // Indicate an end of input and simulation
            }
        }
    }
}

void SequenceInput::update(){
    while(simTime >= NextFrameTime){ // It is time to get a new frame?:
        get_new_frame();
        NextFrameTime += InputFramePeriod; // Update start time of the next frame
    }
}

//------------------------------------------------------------------------------//

// This method returns the last received image which is stored in the output buffer
CImg<double>* SequenceInput::getOutput(){
    return outputImage;
}

//------------------------------------------------------------------------------//

bool SequenceInput::isDummy() {
    return false;
    };