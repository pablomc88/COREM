#include "ShortTermPlasticity.h"

ShortTermPlasticity::ShortTermPlasticity(int x,int y,double temporal_step,double Am,double Vm,double Em, double th, bool isTh){
    slope = Am;
    offset=Vm;
    exponent = Em;

    sizeX = x;
    sizeY = y;
    step = temporal_step;

    kf = 0.0;
    kd = 0.0;
    tau = 1.0;

    if(isTh){
        isThreshold = true;
        threshold = th;
    }else{
        isThreshold = false;
        threshold = 0.0;
    }
}

ShortTermPlasticity::ShortTermPlasticity(const ShortTermPlasticity& copy){

}

ShortTermPlasticity::~ShortTermPlasticity(void){

}

//------------------------------------------------------------------------------//


// Set functions

void ShortTermPlasticity::setSlope(double s){
    slope = s;
}

void ShortTermPlasticity::setOffset(double o){
    offset = o;
}

void ShortTermPlasticity::setExponent(double e){
    exponent = e;
}

void ShortTermPlasticity::setThreshold(double t){
    threshold = t;
    isThreshold = true;
}

void ShortTermPlasticity::setkf(double p1){
    kf = p1;
}

void ShortTermPlasticity::setkd(double p2){
    kd = p2;
}

void ShortTermPlasticity::setTau(double p3){
    tau = p3;
}


//------------------------------------------------------------------------------//

void ShortTermPlasticity::allocateValues(){
    inputImage = new CImg<double>*[7];

    for (int i=0;i<7;i++)
      inputImage[i]=new CImg<double> (sizeY,sizeX,1,1,0.0);


    // exp(-step/tau)
    (inputImage[5])->fill(-step/tau);
    (inputImage[5])->exp();

    outputImage=new CImg<double> (sizeY,sizeX,1,1,0.0);

}

void ShortTermPlasticity::feedInput(double sim_time, const CImg<double>& new_input, bool isCurrent, int port){
    // copy input image
    *inputImage[0] = new_input;
}

void ShortTermPlasticity::update(){

    // kmInf = (kd/(abs(input)))
    // km(t+1) = kmInf + [km(t) - kmInf]*exp(-step/tau)
    // P = P + kf*(km*abs(input) - P)

    // ImagePointer[0] -> input image
    // ImagePointer[1] -> abs(input image)
    // ImagePointer[2] -> P
    // ImagePointer[3] -> km(t)
    // ImagePointer[4] -> kmInf
    // ImagePointer[5] -> exp(-step/tau)
    // ImagePointer[6] -> km(t-1)

    // km(t-1)
    (*(inputImage[6]))=(*(inputImage[3]));
    (*(inputImage[3])).fill(0.0);

    // abs(input)
    *inputImage[1] = *inputImage[0];
    (inputImage[1])->abs();
    (*inputImage[1])+= DBL_EPSILON;

    // kmInf
    (inputImage[4])->fill(kd);
    (inputImage[4])->div((*(inputImage[1])));

    // update of km(t)
    (*inputImage[3]) += (*inputImage[4]);
    (*inputImage[3]) -= (*(inputImage[4])).mul(*(inputImage[5]));
    (*inputImage[3]) += (*(inputImage[6])).mul(*(inputImage[5]));

    // km(t)*abs(input)
    (*inputImage[1]) = *inputImage[0];
    (inputImage[1])->abs();
    (inputImage[1])->mul(*inputImage[3]);

    // update of P
    (*inputImage[2])+= kf*(*inputImage[1] - *inputImage[2]);

    // Threshold
    if(isThreshold){
        cimg_forXY((*inputImage[0]),x,y) {
            if((*inputImage[0])(x,y,0,0) < threshold)
                (*inputImage[0])(x,y,0,0) = threshold;
        }
    }


    // slope, constant offset and exponent
    (*inputImage[0])*=slope;
    (*inputImage[0])+=offset;
    (*inputImage[0])+=(*inputImage[2]);
    inputImage[0]->pow(exponent);

    *outputImage = *inputImage[0];


}

//------------------------------------------------------------------------------//

bool ShortTermPlasticity::setParameters(vector<double> params, vector<string> paramID){

    bool correct = true;

    for (int i = 0;i<params.size();i++){
        const char * s = paramID[i].c_str();

        if (strcmp(s,"slope")==0){
            slope = params[i];
        }else if (strcmp(s,"offset")==0){
            offset = params[i];
        }else if (strcmp(s,"exponent")==0){
            exponent = params[i];
        }
        else if (strcmp(s,"threshold")==0){
            setThreshold(params[i]);
        }
        else if (strcmp(s,"kf")==0){
            setkf(params[i]);
        }
        else if (strcmp(s,"kd")==0){
            setkd(params[i]);
        }
        else if (strcmp(s,"tau")==0){
            setTau(params[i]);
        }
        else{
              correct = false;
        }

    }

    return correct;

}

//------------------------------------------------------------------------------//


CImg<double>* ShortTermPlasticity::getOutput(){
    return outputImage;
}
