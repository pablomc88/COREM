#ifndef SHORTTERMPLASTICITY_H
#define SHORTTERMPLASTICITY_H

/* BeginDocumentation
 * Name: ShortTermPlasticity
 *
 * Description: dynamic nonlinearity that includes a STP term
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 * SeeAlso: module
 */

#include <iostream>
#include <vector>

#include "module.h"

#define DBL_EPSILON_STP 1.0e-2

using namespace cimg_library;
using namespace std;

class ShortTermPlasticity:public module{
protected:
    // nonlinearity parameters
    double slope,offset,exponent,threshold;
    bool isThreshold;
    // STP parameters
    double kf,kd,tau;

    // Buffers
    CImg<double> **inputImage;
    CImg<double> *outputImage;

public:
    // Constructor, copy, destructor.
    ShortTermPlasticity(int x=1,int y=1,double temporal_step=1.0,double Am=1.0,double Vm=0.0,double Em=1.0, double th = 0.0, bool isTh=false);
    ShortTermPlasticity(const ShortTermPlasticity& copy);
    ~ShortTermPlasticity(void);

    // Set functions
    bool setSlope(double s);
    bool setOffset(double o);
    bool setExponent(double e);
    bool setThreshold(double t);

    bool setkf(double p1);
    bool setkd(double p2);
    bool setTau(double p3);
    bool setVInf(double p4);

    // Allocate values
    virtual bool allocateValues();
    // New input and update of equations
    virtual void feedInput(double sim_time, const CImg<double> &new_input, bool isCurrent, int port);
    virtual void update();
    // set Parameters
    virtual bool setParameters(vector<double> params, vector<string> paramID);
    // Get output image (y(k))
    virtual CImg<double>* getOutput();
};


#endif // SHORTTERMPLASTICITY_H
