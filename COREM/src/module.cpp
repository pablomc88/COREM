#include <vector>
#include <string.h>
#include "module.h"

module::module(int x, int y, double temporal_step){
    step = temporal_step;
    sizeX = x;
    sizeY = y;
}

module::module(const module& copy){
    step = copy.step;
    sizeX = copy.sizeX;
    sizeY = copy.sizeY;
}

module::~module(void){
}

//------------------------------------------------------------------------------//
bool module::allocateValues(){
    simTime = 0;
    return(true);
    }

double module::getSizeX(){
    return(sizeX);
}

double module::getSizeY(){
    return(sizeY);
}

bool module::setSizeX(int x){
    bool ret_correct;    
    if (x>0){
        sizeX = x;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool module::setSizeY(int y){
    bool ret_correct;    
    if (y>0){
        sizeY = y;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool module::set_step(double temporal_step) {
    bool ret_correct;    
    if (temporal_step>0){
        step = temporal_step;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

void module::addOperation(vector <int> ops){
    portArith.push_back(ops);
    }

void module::addID(vector <string> ID){
    modulesID.push_back(ID);
    }

vector<int> module::getOperation(int op){
    return portArith[op];
    }
    
vector<string> module::getID(int ID){
    return modulesID[ID];
    }

int module::getSizeID(){
    return modulesID.size();
    }
    
int module::getSizeArith(){
    return portArith.size();
    }

void module::setModuleID(string s){
    ID=s;
    }
    
string module::getModuleID(){
    return ID;
    }

// Set and get synapse type
void module::addTypeSynapse(int type){
    typeSynapse.push_back(type);
    }
    
int module::getTypeSynapse(int port){
    return typeSynapse[port];
    }

bool module::checkID(const char* name){
    const char * charID = ID.c_str();

    if (strcmp(name,charID) == 0){
        return true;
    }else{
        return false;
    }
}

bool module::isDummy() {
    return true;
    }

// Fn definitions just to avoid errors/warnings
void module::feedInput(double sim_time, const CImg<double>& new_input, bool isCurrent, int port){
    }
    
void module::update(){
    }
    
CImg<double>* module::getOutput(){
    return(NULL);
    }
    
bool module::setParameters(vector<double> params, vector<string> paramID){
    return(true);
    }
    
void module::clearParameters(vector<string> paramID){
    }
    
    