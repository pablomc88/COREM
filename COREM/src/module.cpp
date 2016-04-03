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

module& module::setSizeX(int x){
    if (x>0){
        sizeX = x;
    }
}

module& module::setSizeY(int y){
    if (y>0){
        sizeY = y;
    }
}

module& module::set_step(double temporal_step) {
    if (temporal_step>0){
        step = temporal_step;
    }
}
