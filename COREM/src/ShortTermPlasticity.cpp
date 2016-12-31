#include "ShortTermPlasticity.h"

ShortTermPlasticity::ShortTermPlasticity(int x,int y,double temporal_step,double Am,double Vm,double Em, double th, bool isTh):module(x,y,temporal_step){
    slope = Am;
    offset = Vm;
    exponent = Em;

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
    
    inputImage = new CImg<double>*[7];
    for (int i=0;i<7;i++)
        inputImage[i]=new CImg<double>(sizeY,sizeX,1,1,0.0);
    outputImage=new CImg<double> (sizeY,sizeX,1,1,0.0);
}

ShortTermPlasticity::ShortTermPlasticity(const ShortTermPlasticity& copy):module(copy){
    slope=copy.slope;
    offset=copy.offset;
    exponent=exponent;

    kf=copy.kf;
    kd=copy.kd;
    tau=copy.tau;

    isThreshold=copy.isThreshold;
    threshold=copy.threshold;
    
    inputImage = new CImg<double>*[7];
    for (int i=0;i<7;i++)
        inputImage[i]=new CImg<double>(*(copy.inputImage[i]));
    outputImage=new CImg<double> (*(copy.outputImage));
}

ShortTermPlasticity::~ShortTermPlasticity(){
    for (int i=0;i<7;i++)
        delete inputImage[i];
    delete[] inputImage;
    delete outputImage;
}

//------------------------------------------------------------------------------//


// Set functions

bool ShortTermPlasticity::setSlope(double s){
    slope = s;
    return(true);
}

bool ShortTermPlasticity::setOffset(double o){
    offset = o;
    return(true);
}

bool ShortTermPlasticity::setExponent(double e){
    exponent = e;
    return(true);
}

bool ShortTermPlasticity::setThreshold(double t){
    threshold = t;
    isThreshold = true;
    return(true);
}

bool ShortTermPlasticity::setkf(double p1){
    kf = p1;
    return(true);
}

bool ShortTermPlasticity::setkd(double p2){
    kd = p2;
    return(true);
}

bool ShortTermPlasticity::setTau(double p3){
    tau = p3;
    return(true);
}


//------------------------------------------------------------------------------//

bool ShortTermPlasticity::allocateValues(){
    // Resize buffer images to current retina size
    for (int i=0;i<7;i++)
        inputImage[i]->assign(sizeY, sizeX, 1, 1, 0);
    outputImage->assign(sizeY, sizeX, 1, 1, 0);
    
    // exp(-step/tau)
    inputImage[5]->fill(-step/tau);
    inputImage[5]->exp();

    return(true);
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
    (*inputImage[1])+= DBL_EPSILON_STP;

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

    for (vector<double>::size_type i = 0;i < params.size() && correct;i++){
        const char * s = paramID[i].c_str();

        if (strcmp(s,"slope")==0){
            correct = setSlope(params[i]);
        }
        else if (strcmp(s,"offset")==0){
            correct = setOffset(params[i]);
        }
        else if (strcmp(s,"exponent")==0){
            correct = setExponent(params[i]);
        }
        else if (strcmp(s,"threshold")==0){
            correct = setThreshold(params[i]);
        }
        else if (strcmp(s,"kf")==0){
            correct = setkf(params[i]);
        }
        else if (strcmp(s,"kd")==0){
            correct = setkd(params[i]);
        }
        else if (strcmp(s,"tau")==0){
            correct = setTau(params[i]);
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
