
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

StreamingInput::StreamingInput(int x, int y, double temporal_step, string conn_url, unsigned int n_img_reps):module(x,y,temporal_step){
    // Default input parameters
    connection_url = conn_url;    
    frame_repetitons = n_img_reps;
    
    // Input buffer
    outputImage = new CImg<double> (sizeY, sizeX, 1, 1, 0);

    socket_fd = -1; // Socket is not created yet
    
    // Receiver thread has not been created yet, so set its ID to an invalid value
    // For this class an invalid receiver thread value is the ID of the caller thread
    Receiver_thread_id = pthread_self(); 
}

// This method will probably not be used
StreamingInput::StreamingInput(const StreamingInput &copy):module(copy){

    connection_url = copy.connection_url;
    socket_fd = copy.socket_fd;
    frame_repetitons = copy.frame_repetitons;
    Receiver_thread_id = copy.Receiver_thread_id;
    Par = copy.Par;

    outputImage=new CImg<double>(*copy.outputImage);
}

StreamingInput::~StreamingInput(){

    StopImageReception();
    CloseConnection();
    
    delete outputImage;
}

//------------------------------------------------------------------------------//

bool StreamingInput::allocateValues(){
    bool ret_correct;
    module::allocateValues(); // Use the allocateValues() method of the base class
    
    // Resize initial value
    outputImage->assign(sizeY, sizeX, 1, 1, 0);
    
    ret_correct = OpenConnetionPort(); // Set socket to listen
    if(ret_correct)
        ReceiveImages();

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
    // Signal the receiver thread that it can update the value of outputImage buffer
    
}

//------------------------------------------------------------------------------//
#define FIRST_IP_PORT 1
#define LAST_IP_PORT 65535

// This method creates a socket and waits for a connection
bool StreamingInput::OpenConnetionPort(){
    bool ret_correct;
    string url_format = "tcp://localhost:";
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
                    if(listen(socket_fd,1) != -1) {
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

void *image_receiver_thread(void *new_socket_fd)
{
    int socket_fd = (int)(intptr_t)new_socket_fd;
    int read_err;
    
    read_err=0;

    close(socket_fd);
    // The thread will return the error code or 0 if success.
    // we do not know the sizeof(void *) in principle, so cast to intptr_t which has the same sizer to avoid warning
    return((void *)(intptr_t)read_err);
}

bool StreamingInput::ReceiveImages(){
    bool ret_correct;
    struct sockaddr_in cli_addr;
    int new_socket_fd;
    socklen_t cli_addr_len;

    cli_addr_len = sizeof(cli_addr);
    new_socket_fd = accept(socket_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
    if(new_socket_fd != -1){
        int thread_error;
        cout << "Connection from " <<  inet_ntoa(cli_addr.sin_addr) << " accepted!" << endl;
        // Create joinable thread. The thread fn argument will be the socket fd
        thread_error = pthread_create(&Receiver_thread_id, NULL, (void *(*)(void *))&image_receiver_thread, (void *)(intptr_t)new_socket_fd);
        if(thread_error == 0) {// If success
               
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

bool StreamingInput::StopImageReception(){
    bool ret_correct;

    if(pthread_equal(Receiver_thread_id,pthread_self()) == 0) {// Receiver_thread_id <> pthread_self(), so receiver thread was created
        void *thread_ret_ptr;
        int join_err, thread_ret;
        
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

bool StreamingInput::CloseConnection(){
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