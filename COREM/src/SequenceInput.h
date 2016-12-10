#ifndef SEQUENCEINPUT_H
#define SEQUENCEINPUT_H

/* BeginDocumentation
 * Name: SequenceInput
 *
 * Description: Special retina module in charge of obtaining retina input images from a
 *              INR video file or a sequence of image files stored in a directory.
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 * Author: Richard R. Carrillo. University of Granada. CITIC-UGR. Spain.
 *
 * SeeAlso: module
 */

#include <string>
#include <vector>
#include "module.h"

using namespace cimg_library;
using namespace std;

class SequenceInput: public module{
protected:
    // Internal variables
    CImg<double> *outputImage; // Buffer where Update() stores the received image for getOutput()
    double NextFrameTime; // Time at which the next frame must be received
    string InputFilePath; // Path to the INR video file or to the directory contaning the image files 
    unsigned long CurrentInFrameInd; // Number (index) of the input frame to load next
    vector<string> inputFileList; // List of input-file names
    CImg<double> inputMovie; // Volumetric image containing all the input frames
    
    // SequenceInput operation parameters
    int SkipNInitFrames; // Number of of frames to skip just at the beginning of the stream
    bool RepeatLastFrame; // If this parameteris true, the last input frame received is repeated until the end of simulation time
    double InputFramePeriod; // Number of simulation milliseconds that must elapse before a new frame is used. This is an alternative way to specify the FPS of the input.
    bool verbose; // More info menssages printed
public:
    // Constructor, copy, destructor.
    SequenceInput(int x=1, int y=1, double temporal_step=1.0, string input_file_path="");
    SequenceInput(const SequenceInput& copy);
    ~SequenceInput(void);

    // This method opens the specified input (file or directory). If it is a file, it loads the file into memory
    bool openInput();

    // This method closes the input
    void closeInput();
    
    // Skip one frame from input
    void skipFrame();
    
    // Allocate values and set protected parameters
    virtual bool allocateValues();

    // These functions are mainly used by setParameters() to set object parameter properties after the object is created
    bool set_SkipNInitFrames(int n_frames);
    bool set_RepeatLastFrame(bool repeat_flag);
    bool set_InputFramePeriod(double sim_time_period);

    // Only used to update the object simulation time
    virtual void feedInput(double sim_time, const CImg<double> &new_input, bool isCurrent, int port);
    
    // Wait until a new frame is available and update output image buffer
    void get_new_frame();
    
    // update output image according to current sim. time, waiting for a new frame if needed
    virtual void update();
    
    // set Parameters
    virtual bool setParameters(vector<double> params, vector<string> paramID);
    
    // Get image (y(k))
    virtual CImg<double> *getOutput();
    
    // Returns false to indicate that this class performs computation
    virtual bool isDummy();
};

#endif // SEQUENCEINPUT_H
