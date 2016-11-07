#include "module.h"

module::module(int x, int y, double temporal_step){
    step = temporal_step;
    sizeX=x;
    sizeY=y;
}

module::module(const module& copy){
    step = copy.step;
    sizeX=copy.sizeX;
    sizeY=copy.sizeY;
}

module::~module(void){

}

//------------------------------------------------------------------------------//
bool module::allocateValues(){
    simTime=0;
    return(true);
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

bool module::isDummy() {
    return true;
    }