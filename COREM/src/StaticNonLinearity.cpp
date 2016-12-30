#include "StaticNonLinearity.h"

StaticNonLinearity::StaticNonLinearity(int x, int y, double temporal_step, int t){
    sizeX = x;
    sizeY = y;
    step = temporal_step;

    type = t;
    isThreshold = false;
}

StaticNonLinearity::StaticNonLinearity(const StaticNonLinearity& copy){

}

StaticNonLinearity::~StaticNonLinearity(void){

}

//------------------------------------------------------------------------------//


// Set functions
void StaticNonLinearity::setSlope(double s, int segment){
    slope[segment] = s;
}

void StaticNonLinearity::setOffset(double o, int segment){
    offset[segment] = o;
}

void StaticNonLinearity::setExponent(double e, int segment){
    exponent[segment] = e;
}

void StaticNonLinearity::setThreshold(double t, int segment){
    threshold[segment] = t;
    isThreshold = true;
}

void StaticNonLinearity::setType(int t){
    type = t;
}

//------------------------------------------------------------------------------//

void StaticNonLinearity::allocateValues(){
    inputImage=new CImg<double> (sizeY,sizeX,1,1,0.0);
    outputImage=new CImg<double> (sizeY,sizeX,1,1,0.0);
    markers=new CImg<double> (sizeY,sizeX,1,1,0.0);

}

void StaticNonLinearity::feedInput(const CImg<double>& new_input, bool isCurrent, int port){
    // copy input image
    *inputImage = new_input;
}

void StaticNonLinearity::update(){  

    // polynomial function
    if(type==0){

        if(isThreshold){
            cimg_forXY((*inputImage),x,y) {
                if((*inputImage)(x,y,0,0) < threshold[0])
                    (*inputImage)(x,y,0,0) = threshold[0];
            }
        }


        (*inputImage)*=slope[0];
        (*inputImage)+=offset[0];
        inputImage->pow(exponent[0]);
    }

    // piecewise function
    else if(type==1){
            markers->fill(0.0);
            for(int k=0;k<slope.size();k++){
                cimg_forXY((*inputImage),x,y) {
                    if((*inputImage)(x,y,0,0) >= start[k] && (*inputImage)(x,y,0,0) < end[k] && (*markers)(x,y,0,0)==0.0){
                        (*inputImage)(x,y,0,0)*=slope[k];
                        (*inputImage)(x,y,0,0)+=offset[k];
                        (*inputImage)(x,y,0,0) = pow((*inputImage)(x,y,0,0),exponent[k]);
                        (*markers)(x,y,0,0)=1.0;

                    }
                }

            }
    }

    // Symmetric sigmoid (only for negative values)
    else if(type==2){
        double absVal = 0.0;
        cimg_forXY((*inputImage),x,y) {
            absVal = abs((*inputImage)(x,y,0,0));
            (*inputImage)(x,y,0,0) = sgn<double>((*inputImage)(x,y,0,0))*(exponent[0] / (1.0 + exp(-absVal*slope[0] + offset[0])));
        }

    }

    // Standard sigmoid
    else if(type==3){
        double value = 0.0;
        cimg_forXY((*inputImage),x,y) {
            value = (*inputImage)(x,y,0,0);
            (*inputImage)(x,y,0,0) = (exponent[0] / (1.0 + exp(-value*slope[0] + offset[0])));
        }

    }

    *outputImage = *inputImage;

}

//------------------------------------------------------------------------------//

bool StaticNonLinearity::setParameters(vector<double> params, vector<string> paramID){

    bool correct = true;

    for (int i = 0;i<params.size();i++){
        const char * s = paramID[i].c_str();

        if (strcmp(s,"slope")==0){
            slope.push_back(params[i]);
        }else if (strcmp(s,"offset")==0){
            offset.push_back(params[i]);
        }else if (strcmp(s,"exponent")==0){
            exponent.push_back(params[i]);
        }else if (strcmp(s,"max")==0){
            exponent.push_back(params[i]);
        }else if (strcmp(s,"threshold")==0){
            threshold.push_back(params[i]);
            isThreshold = true;
        }
        else if (strcmp(s,"start")==0){
            start.push_back(params[i]);
        }
        else if (strcmp(s,"end")==0){
            end.push_back(params[i]);
        }
        else{
              correct = false;
        }

    }

    return correct;

}

void StaticNonLinearity::clearParameters(vector<string> paramID){

    for (int i = 0;i<paramID.size();i++){
        const char * s = paramID[i].c_str();

        if (strcmp(s,"slope")==0){
            slope.clear();
        }else if (strcmp(s,"offset")==0){
            offset.clear();
        }else if (strcmp(s,"exponent")==0){
            exponent.clear();
        }else if (strcmp(s,"max")==0){
            exponent.clear();
        }else if (strcmp(s,"threshold")==0){
            threshold.clear();
        }
        else if (strcmp(s,"start")==0){
            start.clear();
        }
        else if (strcmp(s,"end")==0){
            end.clear();
        }


    }



}

//------------------------------------------------------------------------------//


CImg<double>* StaticNonLinearity::getOutput(){
    return outputImage;
}

//------------------------------------------------------------------------------//

template <typename T> int StaticNonLinearity::sgn(T val) {
    return (T(0) < val) - (val < T(0));
}
