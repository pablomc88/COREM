#ifndef WHITENOISE_H
#define WHITENOISE_H

/* BeginDocumentation
 * Name: whiteNoise
 *
 * Description: White Noise generator.
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 * SeeAlso: GratingGenerator, fixationalMovGrating, impulse
 */

#include "../CImg-1.6.0_rolling141127/CImg.h"

#include <random>
#include <chrono>

using namespace cimg_library;
using namespace std;

class whiteNoise{
private:

    // Normal distributions
    normal_distribution<double> distribution1;
    default_random_engine generator1;
    normal_distribution<double> distribution2;
    default_random_engine generator2;

    // time to switch
    double switchTime;

    // new value
    double GaussianPeriod;

    // Output image
    CImg <double> *output;

public:
    // Constructor, copy, destructor.
    whiteNoise();
    whiteNoise(double mean, double contrast1, double contrast2, double period, double switchT,int X, int Y);
    whiteNoise(const whiteNoise& copy);
    ~whiteNoise(void);

    // update
    CImg<double>* update(double t);

    // initialize distributions
    void initializeDist(unsigned seed);

    // get time to switch
    double getSwitchTime(){return switchTime;}
};

#endif // WHITENOISE_H
