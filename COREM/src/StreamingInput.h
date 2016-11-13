#ifndef STREAMINGINPUT_H
#define STREAMINGINPUT_H

/* BeginDocumentation
 * Name: StreamingInput
 *
 * Description: Special retina module in charge of obtaining the retina input images from a
 *              video streaming through a network connection.
 * In particular it creates a TCP socket listening for a incoming connection through which the
 * input images are received in PNG format sequentialy.
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
#include <pthread.h>
#include "module.h"

using namespace cimg_library;
using namespace std;


class StreamingInput: public module{
protected:
    // image buffers
    //CImg<double> *bufferImage; // Buffer used to temporally store the image being received
    CImg<double> *outputImage; // Buffer where Update() stores the image for getOutput()
    int socket_fd; // Connection socket file descriptor or -1 if socket has not been creted
    string connection_url; // URL of the connection. It must be 'tcp://localhost:port', where port 
    pthread_t Receiver_thread_id; // ID of the thread created to receive images
    unsigned int frame_repetitons;

    double Par; // to be erased
public:
    // Constructor, copy, destructor.
    StreamingInput(int x=1, int y=1, double temporal_step=1.0, string conn_url="", unsigned int n_img_reps=1);
    StreamingInput(const StreamingInput& copy);
    ~StreamingInput(void);

    // Allocate values and set protected parameters
    virtual bool allocateValues();

    // These functions are mainly used by setParameters() to set object parameter properties after the object is created
    bool set_Par(double par);

    // Get new input
    virtual void feedInput(double sim_time, const CImg<double> &new_input, bool isCurrent, int port);
    // update of state and write output frame to file
    virtual void update();
    // set Parameters
    virtual bool setParameters(vector<double> params, vector<string> paramID);
    
    // This method creates a socket (used for streaming) and bind it to a local port
    bool OpenConnetionPort();

    // This method waits for an incomming connection and creates a thread which
    // continuously receives images
    bool ReceiveImages();

    // This method stops the thread in charge of receving the images
    bool StopImageReception();
    
    // This method closes the streaming connection
    bool CloseConnection();
    
    // Get output image (y(k)) (not used)
    virtual CImg<double>* getOutput();
    
    // Returns false to indicate that this class performs computation
    virtual bool isDummy();
};

#endif // STREAMINGINPUT_H
