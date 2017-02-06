
#include <iostream>

// For file writing:
#include <fstream>
#include <string>
#include <stdio.h>

#include <limits>

#include <cerrno>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "StreamingInput.h"

StreamingInput::StreamingInput(int x, int y, double temporal_step, string conn_url):module(x,y,temporal_step){
    pthread_mutexattr_t mutex_attrib;
    
    // Default input parameters
    connection_url = conn_url;    
    SkipNInitFrames = 0; // No frame skipped by default
    RepeatLastFrame = false; // Sim. is terminted after end of input
    InputFramePeriod = 1; // by default one new frame is used each simulation millisecond

    // Allocate image buffers buffer
    outputImage = new CImg<double> (sizeY, sizeX, 1, 1, 0);
    receiver_vars.buffer_img = new CImg<double> (sizeY, sizeX, 1, 1, 0);

    // Init. internal vars
    NextFrameTime = 0; // First frame must be received at time 0
    socket_fd = -1; // There is no connection at the beginning, so the Sockets and stream are not created yet
    accept_socket_fd = -1;
    receiver_vars.accept_socket_fh = NULL;
    
    receiver_vars.exit_reception = false; // Exit has not been signaled
    
    // Set specific mutex attributes: If a thread attempts to unlock an unlocked the fn just returns an error
    // If the a thread attemps to relock a mutex just an error is returned (no deadlock occurs).
    pthread_mutexattr_init(&mutex_attrib);
    pthread_mutexattr_settype(&mutex_attrib, PTHREAD_MUTEX_NORMAL); // No mutex owner checking
    pthread_mutex_init(&receiver_vars.buffer_mutex, &mutex_attrib); // Initialize buffer mutex
    pthread_mutex_init(&receiver_vars.reception_mutex, &mutex_attrib); // Initialize reception mutex
    pthread_mutexattr_destroy(&mutex_attrib); // We do not need the attributes: destroy them

    // This implementation for mutual exclusion of access to receiver_vars.buffer_img is really not legal, because
    // a thread should not unlock a mutex which has been locked by other thread, however, it does work because
    // mutex are created with attibute PTHREAD_MUTEX_NORMAL or (PTHREAD_MUTEX_DEFAULT), so the thread ID is not checked.
    // However, a legal implementation should use pthread_cond_wait to alternate the access to the buffer.
    
    // Receiver thread has not been created yet, so set its ID to an invalid value
    // For this class an invalid receiver thread value is the ID of the caller thread
    Receiver_thread_id = pthread_self(); 
}

// This method will probably not be used
StreamingInput::StreamingInput(const StreamingInput &copy):module(copy){

    connection_url = copy.connection_url;
    socket_fd = copy.socket_fd;
    accept_socket_fd = copy.accept_socket_fd;
    receiver_vars.accept_socket_fh = copy.receiver_vars.accept_socket_fh;
    Receiver_thread_id = copy.Receiver_thread_id;
    receiver_vars = copy.receiver_vars;
    receiver_vars.buffer_mutex = copy.receiver_vars.buffer_mutex;
    receiver_vars.reception_mutex = copy.receiver_vars.reception_mutex;
    SkipNInitFrames = copy.SkipNInitFrames;
    RepeatLastFrame = copy.RepeatLastFrame;
    InputFramePeriod = copy.InputFramePeriod;
    NextFrameTime = copy.NextFrameTime;

    outputImage=new CImg<double>(*copy.outputImage);
    receiver_vars.buffer_img=new CImg<double>(*copy.receiver_vars.buffer_img);
}

StreamingInput::~StreamingInput(){

    stopStreamReception();
    closeConnection();

    pthread_mutex_unlock(&receiver_vars.reception_mutex); // Attempting to destroy a locked mutex results in undefined behaviour
    pthread_mutex_unlock(&receiver_vars.buffer_mutex);
    pthread_mutex_destroy(&receiver_vars.reception_mutex);
    pthread_mutex_destroy(&receiver_vars.buffer_mutex);
    
    if(receiver_vars.buffer_img != NULL)
        delete receiver_vars.buffer_img;
    if(outputImage != NULL)
        delete outputImage;
}

//------------------------------------------------------------------------------//

bool StreamingInput::allocateValues(){
    bool ret_correct;
    module::allocateValues(); // Call the allocateValues() method of the base class
    
    ret_correct = openConnetion(); // Set socket to listen and wait for a connection
    if(ret_correct){
        if(SkipNInitFrames>0)
            cout << "Skipping " << SkipNInitFrames << " input frames" << endl;
        for(int n_skipped_frames=0;n_skipped_frames<SkipNInitFrames;n_skipped_frames++)
            outputImage->load_png(receiver_vars.accept_socket_fh); // Skip frame
            
        // Use the first frame to find out the new dimensions of retina image size
        outputImage->load_png(receiver_vars.accept_socket_fh); // Get first valid frame
        sizeY=outputImage->width();
        sizeX=outputImage->height();
        NextFrameTime=InputFramePeriod; // Next frame must be read at this time
        // output image should have been automatically resized after first frame load
        // Resize buffer image as well
        receiver_vars.buffer_img->assign(sizeY, sizeX, 1, 1, 0);
        
        // The first frame is now ready in the output, but receicer thread can still populate the buffer image,
        // so leave the receiver unlocked (as it is by default) and lock the access to the buffer 
        pthread_mutex_lock(&receiver_vars.buffer_mutex); // lock access the the buffer (it is not full yet)
        
        ret_correct = receiveStream();
    }

    return(ret_correct);
}

//------------------------------------------------------------------------------//

bool StreamingInput::set_SkipNInitFrames(int n_frames){
    bool ret_correct;
    if (n_frames>=0) {
        SkipNInitFrames = n_frames;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool StreamingInput::set_RepeatLastFrame(bool repeat_flag){
    RepeatLastFrame = repeat_flag;
    return(true);
}

bool StreamingInput::set_InputFramePeriod(double sim_time_period){
    bool ret_correct;
    if (sim_time_period>0) {
        InputFramePeriod = sim_time_period;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

//------------------------------------------------------------------------------//

bool StreamingInput::setParameters(vector<double> params, vector<string> paramID){

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
void StreamingInput::feedInput(double sim_time, const CImg<double> &new_input, bool isCurrent, int port){
    // Update the current simulation time (although it is currently not used)
    simTime = sim_time;
}


//------------------------------------------------------------------------------//

void StreamingInput::get_new_frame(){
    if(receiver_vars.buffer_img != NULL) // Don't wait for a frame if input already ended
        pthread_mutex_lock(&receiver_vars.buffer_mutex); // Waits until a new frame is ready

    if(receiver_vars.buffer_img != NULL) {
        *outputImage = *receiver_vars.buffer_img; // Get output from buffer
        pthread_mutex_unlock(&receiver_vars.reception_mutex); // Signal the receiver thread that it can update the value of buffer_img buffer with a new frame
    } else { // End of stream
        if(outputImage != NULL && !RepeatLastFrame){
            delete outputImage;
            outputImage=NULL; // Indicate an end of input ans simulation
        }
    }
}

void StreamingInput::update(){
    while(simTime >= NextFrameTime){ // It is time to get a new frame?:
        get_new_frame();
        NextFrameTime += InputFramePeriod; // Update start time of the next frame
    }
}

//------------------------------------------------------------------------------//
#define FIRST_IP_PORT 1
#define LAST_IP_PORT 65535

// This method creates a socket and waits for a connection
bool StreamingInput::openConnetion(){
    bool ret_correct;
    string url_format = "tcp://passive:";
    size_t url_format_len = url_format.size();

    // Check whether the beginning of the specified connection_url is equal to url_format
    if(connection_url.compare(0, url_format_len, url_format) == 0) {
        string port_str = connection_url.substr(url_format_len);
        int port_number = stoi(port_str);
        
        if(port_number >= FIRST_IP_PORT && port_number <= LAST_IP_PORT) { // Valid port numbers
            socket_fd = socket(AF_INET, SOCK_STREAM, 0);
            if(socket_fd != -1) {
                struct sockaddr_in serv_addr;
                
                bzero((void *)&serv_addr, sizeof(serv_addr));
                serv_addr.sin_family = AF_INET;
                serv_addr.sin_port = htons(port_number);
                serv_addr.sin_addr.s_addr = INADDR_ANY;
                if(bind(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != -1) {
                    if(listen(socket_fd, 1) != -1) {
                        struct sockaddr_in cli_addr;
                        socklen_t cli_addr_len;

                        cout << "Waiting for an incoming connection at port " << port_number << "..." << endl;
                        cli_addr_len = sizeof(cli_addr);
                        accept_socket_fd = accept(socket_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
                        if(accept_socket_fd != -1) {
                            cout << "Connection from " <<  inet_ntoa(cli_addr.sin_addr) << " accepted!" << endl;
                            receiver_vars.accept_socket_fh = fdopen(accept_socket_fd, "rb"); // Open the socket as a file stream
                            if(receiver_vars.accept_socket_fh != NULL) {
                                ret_correct=true;
                            } else {
                                cout << "Error opening accept socket as a file stream. errno: " << errno << endl;
                                close(accept_socket_fd);
                                accept_socket_fd=-1;
                                close(socket_fd);
                                socket_fd=-1;
                                ret_correct = false;
                            }
                        }
                        else{
                            cout << "Error accepting incomming connection. errno: " << errno << endl;
                            close(socket_fd);
                            socket_fd=-1;
                            ret_correct = false;
                        }
                    } else {
                        cout << "Connection socket could not set to passive socket in port " << port_number << ". errno: " << errno << "." << endl;
                        close(socket_fd);
                        socket_fd=-1;
                        ret_correct = false;
                    }
                } else {
                    cout << "Connection socket could not be binded to port " << port_number << ". errno: " << errno << "." << endl;
                    close(socket_fd);
                    socket_fd=-1;
                    ret_correct = false;
                }
            } else {
                cout << "Error creating connection socket. errno: " << errno << "." << endl;
                ret_correct = false;
            }
        } else {
            cout << "Incorrect connection port number specified: " << port_number << ". Expected a number in [" << FIRST_IP_PORT << "," << LAST_IP_PORT << "]." << endl;
            ret_correct = false;
        }
    } else {
        cout << "Incorrect connection URL format specified: " << connection_url << ". Expected: " << url_format << "port_number." << endl;
        ret_correct = false;
    }
    return(ret_correct);
}

void *image_receiver_thread(struct receiver_params *params)
{
    int error_code;
    
    error_code=0;
    while(!params->exit_reception && error_code==0) {
        error_code=pthread_mutex_lock(&(params->reception_mutex)); // Waits until reception is allowed (buffer has been consumed)
        if(!params->exit_reception && error_code==0){
            int first_frame_char;
            // Try to get the last frame character to check if a next frame is being sent
            first_frame_char=fgetc(params->accept_socket_fh);
            if(first_frame_char != EOF) { // Another frame is comming
                ungetc(first_frame_char, params->accept_socket_fh); // Put the char back in the stream as that a complete image can be loaded
                
                params->buffer_img->load_png(params->accept_socket_fh); // Receive a complete frame

                error_code=pthread_mutex_unlock(&(params->buffer_mutex)); // New frame available, unblock update()
            } else {
                pthread_mutex_unlock(&(params->buffer_mutex)); // New (empty) frame available, unblock update()
                error_code=EIO; // Exit loop
                delete params->buffer_img;
                params->buffer_img=NULL; // End of stream is indicated with a NULL pointer frame
            }
        }
    }
    // The thread will return the error code or 0 if success.
    if(error_code==EIO)
        cout << "\rStreaming connection was closed by the other end." << endl;
    // we do not know the sizeof(void *) in principle, so cast to intptr_t which has the same sizer to avoid warning
    return((void *)(intptr_t)error_code);
}

bool StreamingInput::receiveStream(){
    bool ret_correct;
    int thread_error;

    // Create joinable thread. The thread fn argument will be the socket fd
    thread_error = pthread_create(&Receiver_thread_id, NULL, (void *(*)(void *))&image_receiver_thread, (void *)&receiver_vars);
    if(thread_error == 0) { // If success
        ret_correct = true;
    } else {
        cout << "Failed to create image receiver thread. return error code: " << thread_error << "." << endl;
        ret_correct = false;
    }
    return(ret_correct);
}

bool StreamingInput::stopStreamReception(){
    bool ret_correct;

    if(pthread_equal(Receiver_thread_id,pthread_self()) == 0) {// Receiver_thread_id <> pthread_self(), so receiver thread was created
        void *thread_ret_ptr;
        int join_err, thread_ret;
        receiver_vars.exit_reception = true; // Signal the receiver thread to terminate
        pthread_mutex_unlock(&receiver_vars.reception_mutex); // Unlock reception (just in case the thread was locked)
        join_err = pthread_join(Receiver_thread_id, &thread_ret_ptr); // Wait for thread to terminate
        if(join_err == 0) {// If success joining
            thread_ret = (int)(intptr_t)thread_ret_ptr;
            if(thread_ret != 0){
                if(thread_ret != EIO)
                    cout << "Frame reception ended anormally. errno: " << thread_ret << "." << endl;
            }
            
            ret_correct = true;
        } else {
            cout << "Error waiting for the image receiver thread to finish. return error code: " << join_err << endl;
            ret_correct = false;
        }
    // pthread_cancel(
    } else
        ret_correct = true;
        
    return(ret_correct);
}

bool StreamingInput::closeConnection(){
    bool ret_correct;

    if(receiver_vars.accept_socket_fh != NULL)
        fclose(receiver_vars.accept_socket_fh); // Close file handle and correponding file descriptor
    else {
        if(accept_socket_fd != -1)
            close(accept_socket_fd);
    }

    if(socket_fd != -1){ // Check if the socket was created
        ret_correct = (close(socket_fd) == 0);
    }
    else
        ret_correct = true;
    return(ret_correct);
}

//------------------------------------------------------------------------------//

// This method returns the last received image which is stored in the output buffer
CImg<double>* StreamingInput::getOutput(){
    return outputImage;
}

//------------------------------------------------------------------------------//

bool StreamingInput::isDummy() {
    return false;
    };