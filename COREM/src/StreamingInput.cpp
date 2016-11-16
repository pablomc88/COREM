
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
    
    // Allocate image buffers buffer
    outputImage = new CImg<double> (sizeY, sizeX, 1, 1, 0);
    receiver_vars.buffer_img = new CImg<double> (sizeY, sizeX, 1, 1, 0);

    socket_fd = -1; // Socket is not created yet
    receiver_vars.exit_reception = false; // Exit has not been signaled
    
    // Set specific mutex attributes: If a thread attempts to unlock an unlocked the fn just returns an error
    // If the a thread attemps to relock a mutex just an error is returned (no deadlock occurs).
    pthread_mutexattr_init(&mutex_attrib);
    pthread_mutexattr_settype(&mutex_attrib, PTHREAD_MUTEX_NORMAL); // No mutex owner checking
    pthread_mutex_init(&receiver_vars.buffer_mutex, &mutex_attrib); // Initialize buffer mutex
    pthread_mutex_init(&receiver_vars.reception_mutex, &mutex_attrib); // Initialize reception mutex
    pthread_mutexattr_destroy(&mutex_attrib); // We do not need the attributes: destroy them

    pthread_mutex_lock(&receiver_vars.buffer_mutex); // buffer_img is not ready until unlocked by the receiver thread
    // Receiver thread has not been created yet, so set its ID to an invalid value
    // For this class an invalid receiver thread value is the ID of the caller thread
    Receiver_thread_id = pthread_self(); 
}

// This method will probably not be used
StreamingInput::StreamingInput(const StreamingInput &copy):module(copy){

    connection_url = copy.connection_url;
    socket_fd = copy.socket_fd;
    Receiver_thread_id = copy.Receiver_thread_id;
    receiver_vars = copy.receiver_vars;
    Par = copy.Par;

    outputImage=new CImg<double>(*copy.outputImage);
    receiver_vars.buffer_img=new CImg<double>(*copy.receiver_vars.buffer_img);
}

StreamingInput::~StreamingInput(){

    stopImageReception();
    closeConnection();

    pthread_mutex_unlock(&receiver_vars.reception_mutex); // Attempting to destroy a locked mutex results in undefined behaviour
    pthread_mutex_unlock(&receiver_vars.buffer_mutex);
    pthread_mutex_destroy(&receiver_vars.reception_mutex);
    pthread_mutex_destroy(&receiver_vars.buffer_mutex);
    
    delete receiver_vars.buffer_img;
    delete outputImage;
}

//------------------------------------------------------------------------------//

bool StreamingInput::allocateValues(){
    bool ret_correct;
    module::allocateValues(); // Use the allocateValues() method of the base class
    
    // Resize initial value
    outputImage->assign(sizeY, sizeX, 1, 1, 0);
    
    ret_correct = openConnetionPort(); // Set socket to listen
    if(ret_correct)
        receiveImages();

    return(ret_correct);
}


bool StreamingInput::set_Par(double par){
    bool ret_correct;
    if (par>=0) {
        Par = par;
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

        if (strcmp(s,"Par")==0){
            correct = set_Par(params[i]);
        } else{
              correct = false;
        }
    }
    return correct;
}

//------------------------------------------------------------------------------//

// This method can only be used to set the simulation time
void StreamingInput::feedInput(double sim_time, const CImg<double>& new_input,bool isCurrent,int port){
    // Update the current simulation time (although it is currently not used)
    simTime = sim_time;
}


//------------------------------------------------------------------------------//

void StreamingInput::update(){
    pthread_mutex_lock(&receiver_vars.buffer_mutex); // Waits until a new frame is ready
    *outputImage = *receiver_vars.buffer_img; // Get output from buffer
    pthread_mutex_unlock(&receiver_vars.reception_mutex); // Signal the receiver thread that it can update the value of buffer_img buffer with a new frame
}

//------------------------------------------------------------------------------//
#define FIRST_IP_PORT 1
#define LAST_IP_PORT 65535

// This method creates a socket and waits for a connection
bool StreamingInput::openConnetionPort(){
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
                        ret_correct=true;
                    } else {
                        cout << "Connection socket could not set to passive socket in port " << port_number << ". errno: " << errno << "." << endl;
                        ret_correct = false;
                    }
                } else {
                    cout << "Connection socket could not be binded to port " << port_number << ". errno: " << errno << "." << endl;
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
    FILE *acc_socket_fh;
    
    cout << "socket: " << params->accept_socket_fd << endl;
    error_code=0;
    acc_socket_fh = fdopen(params->accept_socket_fd, "rb");
    if(acc_socket_fh != NULL)
    {
        while(!params->exit_reception && error_code==0) {
            error_code=pthread_mutex_lock(&(params->reception_mutex)); // Waits until reception is allowed (buffer has been consumed)
            if(!params->exit_reception && error_code==0){
                //cout << "Receiving... " << endl;
                
                params->buffer_img->load_png(acc_socket_fh);
                cout << "." << flush;
                //cout << "Received " << params->buffer_img->width() << " x " << params->buffer_img->height() << endl;
                error_code=pthread_mutex_unlock(&(params->buffer_mutex)); // New frame available, unblock update()
            }
        }
        fclose(acc_socket_fh); // Close file handle and correponding file descriptor
    } else {
        close(params->accept_socket_fd);
        error_code=errno;
    }
        
    cout << "eeror code: " << error_code << endl;
    // The thread will return the error code or 0 if success.
    // we do not know the sizeof(void *) in principle, so cast to intptr_t which has the same sizer to avoid warning
    return((void *)(intptr_t)error_code);
}

bool StreamingInput::receiveImages(){
    bool ret_correct;
    struct sockaddr_in cli_addr;
    socklen_t cli_addr_len;

    cout << "Waiting for incomming connection..." << endl;
    cli_addr_len = sizeof(cli_addr);
    receiver_vars.accept_socket_fd = accept(socket_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
    if(receiver_vars.accept_socket_fd != -1){
        int thread_error;
        cout << "Connection from " <<  inet_ntoa(cli_addr.sin_addr) << " accepted!" << endl;
        // Create joinable thread. The thread fn argument will be the socket fd
        thread_error = pthread_create(&Receiver_thread_id, NULL, (void *(*)(void *))&image_receiver_thread, (void *)&receiver_vars);
        if(thread_error == 0) { // If success
            
        } else {
            cout << "Failed to create image receiver thread. return error code: " << thread_error << "." << endl;
            ret_correct = false;
        }
    }
    else{
        cout << "Error accepting incomming connection. errno: " << errno << endl;
        ret_correct = false;
    }
    return(ret_correct);
}

bool StreamingInput::stopImageReception(){
    bool ret_correct;

    if(pthread_equal(Receiver_thread_id,pthread_self()) == 0) {// Receiver_thread_id <> pthread_self(), so receiver thread was created
        void *thread_ret_ptr;
        int join_err, thread_ret;
        receiver_vars.exit_reception = true; // Signal the receiver thread to terminate
        pthread_mutex_unlock(&receiver_vars.reception_mutex); // Unlock reception (just in case)
        join_err = pthread_join(Receiver_thread_id, &thread_ret_ptr);
        if(join_err == 0) {// If success joining
            thread_ret = (int)(intptr_t)thread_ret_ptr;
            if(thread_ret != 0)
                cout << "Image receiver thread ended anormally. code: " << thread_ret << "." << endl;
            
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