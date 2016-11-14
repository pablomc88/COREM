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

// Parameter of frame receiver thread
struct receiver_params {
    int accept_socket_fd; // Socket fd created when a connection is accepted
    CImg<double> *buffer_img; // Buffer used to temporally store the image being received
    bool exit_reception; // Reception threads exits when this var is set to true by a the caller 
    pthread_mutex_t buffer_mutex; // This var is locked when the buffer is started to be copied to the output and unlocked when it can be copied agin (new frame received)
    pthread_mutex_t reception_mutex; // This var is locked when the thread starts receiving and unlocked the when can receive again (buffer copied to output)
};
// Thread function in charge of receiving frames through socket (int)new_socket_fd and
// storing them in bufferImage.
// After soreing a frame it signals the completion and waits until the signal is cleared
// to sore the next one.
void *image_receiver_thread(struct receiver_params *params);

class StreamingInput: public module{
protected:
    // image output
    CImg<double> *outputImage; // Buffer where Update() stores the received image for getOutput()
    int socket_fd; // Connection socket file descriptor or -1 if socket has not been creted
    string connection_url; // URL of the connection. It must be 'tcp://localhost:port', where port 
    pthread_t Receiver_thread_id; // ID of the thread created to receive images
    struct receiver_params receiver_vars; // Variables shared between the class object and the thread
    double Par; // to be erased
public:
    // Constructor, copy, destructor.
    StreamingInput(int x=1, int y=1, double temporal_step=1.0, string conn_url="");
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
    bool openConnetionPort();

    // This method waits for an incomming connection and creates a thread which
    // continuously receives images
    bool receiveImages();

    // This method stops the thread in charge of receving the images
    bool stopImageReception();
    
    // This method closes the streaming connection
    bool closeConnection();
    
    // Get output image (y(k)) (not used)
    virtual CImg<double>* getOutput();
    
    // Returns false to indicate that this class performs computation
    virtual bool isDummy();
};

#endif // STREAMINGINPUT_H
