#include "impulse.h"

impulse::impulse(double startParam, double stopParam, double amplitudeParam, double offsetParam,int X, int Y){
    start = startParam;
    stop = stopParam;
    amplitude = amplitudeParam;
    offset = offsetParam;

    output = new CImg <double>(Y,X,1,3);

    cimg_forXY(*output,x,y) {
        (*output)(x,y,0,0) = 1.0,
        (*output)(x,y,0,1) = 1.0,
        (*output)(x,y,0,2) = 1.0;
    }
}

impulse::impulse(const impulse& copy){

}

impulse::~impulse(void){

}

//------------------------------------------------------------------------------//

CImg<double>* impulse::update(double t){


    if(t>=start && t <=stop){
        cimg_forXY(*output,x,y) {
            (*output)(x,y,0,0) = amplitude + offset,
            (*output)(x,y,0,1) = amplitude + offset,
            (*output)(x,y,0,2) = amplitude + offset;
        }
    }else{
        cimg_forXY(*output,x,y) {
            (*output)(x,y,0,0) = offset,
            (*output)(x,y,0,1) = offset,
            (*output)(x,y,0,2) = offset;
        }
    }


    return output;
}
