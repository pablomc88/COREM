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

#define DBL_EPSILON 1.0e-2

using namespace cimg_library;
using namespace std;

class ShortTermPlasticity:public module{
protected:
    // nonlinearity parameters
    double slope,offset,exponent,threshold;
    bool isThreshold;
    // STP parameters
    double kf,kd,tau;
    // Simulation step
    double step;

    // Buffers
    CImg<double> **inputImage;
    CImg<double> *outputImage;

public:
    // Constructor, copy, destructor.
    ShortTermPlasticity(int x=1,int y=1,double temporal_step=1.0,double Am=1.0,double Vm=0.0,double Em=1.0, double th = 0.0, bool isTh=false);
    ShortTermPlasticity(const ShortTermPlasticity& copy);
    ~ShortTermPlasticity(void);

    // Set functions
    void setSlope(double s);
    void setOffset(double o);
    void setExponent(double e);
    void setThreshold(double t);

    void setkf(double p1);
    void setkd(double p2);
    void setTau(double p3);
    void setVInf(double p4);

    // Allocate values
    virtual void allocateValues();
    virtual void setX(int x){sizeX=x;}
    virtual void setY(int y){sizeY=y;}
    // New input and update of equations
    virtual void feedInput(const CImg<double> &new_input, bool isCurrent, int port);
    virtual void update();
    // set Parameters
    virtual bool setParameters(vector<double> params, vector<string> paramID);
    // Get output image (y(k))
    virtual CImg<double>* getOutput();
};


#endif // SHORTTERMPLASTICITY_H
