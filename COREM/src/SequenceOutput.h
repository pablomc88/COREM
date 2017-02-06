#ifndef SEQUENCEOUTPUT_H
#define SEQUENCEOUTPUT_H

/* BeginDocumentation
 * Name: SequenceOutput
 *
 * Description: Special retina module in charge of saving the retina output in a file as a image sequence.
 * In particular it can create an INR video file containing one image per simulation time step.
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 * Author: Richard R. Carrillo. University of Granada. CITIC-UGR. Spain.
 *
 * SeeAlso: module
 */

#include <fstream>
#include <vector>
#include <string>
#include "module.h"

using namespace cimg_library;
using namespace std;

#define INR_HEADER_LEN 256 // Header length of a INRIMAGE-4 file

class SequenceOutput:public module{
protected:
    // image buffers
    CImg<double> *inputImage; // Buffer used to temporally store the input values which will be saved

    string out_seq_filename; // filename (including path) to the movie output file to create
    ofstream out_seq_file_handle; // The out_seq_filename file is created when the object is created and this handle is set
    unsigned int num_written_frames; // Number of frames currently added to out_seq_file_handle
    unsigned int num_skipped_frames; // Number of frames currently skipped (following user specification)

    double Start_time, End_time; // These recording parameters define the simulation time interval when the images must be saved
    
    // parameters of output file
    double Voxel_X_size, Voxel_Y_size; // Size of a voxel (pixel) in X and Y dimensions. The size in Z (time) is always set to 1
    unsigned int InFramesPerOut; // Number of input frames waited until an output frame is generated
public:
    // Constructor, copy, destructor.
    SequenceOutput(int x=1, int y=1, double temporal_step=1.0, string output_filename="");
    SequenceOutput(const SequenceOutput& copy);
    ~SequenceOutput(void);

    // Allocate values and set protected parameters
    virtual bool allocateValues();

    // These functions are mainly used by setParameters() to set object parameter properties after the object is created
    bool set_Voxel_X_size(double voxel_x_size);
    bool set_Voxel_Y_size(double voxel_y_size);
    bool set_Start_time(double start_time);
    bool set_End_time(double end_time);
    bool set_InFramesPerOut(unsigned int n_frames);

    // Get new input
    virtual void feedInput(double sim_time, const CImg<double> &new_input, bool isCurrent, int port);
    // update of state and write output frame to file
    virtual void update();
    // set Parameters
    virtual bool setParameters(vector<double> params, vector<string> paramID);
    
    // Return a pointer to a string identifying the endianness of the architecture in which
    // the method is being executed: "pc" or "sun"
    const char *getEndianness();
    
    // This method creates (or overwrites) out_spk_filename with an empty header
    // It returns true when it could be successfully written
    bool CreateINRFile();

    // Save the input buffer into a file as a new frame
    bool WriteINRFrame();
    
    // Write a file header with INRIMAGE-4 format in out_spk_filename considering
    // the current class properties
    bool CloseINRFile();
    
    // Get output image (y(k)) (not used)
    virtual CImg<double>* getOutput();
    
    // Returns false to indicate that this class performs computation
    virtual bool isDummy();
};

#endif // SEQUENCEOUTPUT_H
