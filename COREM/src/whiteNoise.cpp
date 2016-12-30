#include "whiteNoise.h"

whiteNoise::whiteNoise(double mean, double contrast1, double contrast2, double period, double switchT,int X, int Y){

    switchTime = switchT;
    GaussianPeriod = period;

    distribution1 = *(new normal_distribution<double>(mean,contrast1*mean));
    distribution2 = *(new normal_distribution<double>(mean,contrast2*mean));

    output = new CImg <double>(Y,X,1,3);

    cimg_forXY(*output,x,y) {
        (*output)(x,y,0,0) = 1.0,
        (*output)(x,y,0,1) = 1.0,
        (*output)(x,y,0,2) = 1.0;
    }

}

whiteNoise::whiteNoise(const whiteNoise& copy){

}

whiteNoise::~whiteNoise(void){

}

//------------------------------------------------------------------------------//

void whiteNoise::initializeDist(unsigned seed){
    generator1 = *(new default_random_engine(seed));
    generator2 = *(new default_random_engine(seed));

}

//------------------------------------------------------------------------------//


CImg<double>* whiteNoise::update(double t){

    // draw new value from Gaussian distribution
    if((int)t%(int)GaussianPeriod == 0){

        double value = 0;
        if(t < switchTime)
            value = distribution1(generator1);
        else
            value = distribution2(generator2);

        if(value<0.0)
            value = 0.0;

        cimg_forXY(*output,x,y) {
            (*output)(x,y,0,0) = value*255,
            (*output)(x,y,0,1) = value*255,
            (*output)(x,y,0,2) = value*255;
        }

    }

    return output;
}
