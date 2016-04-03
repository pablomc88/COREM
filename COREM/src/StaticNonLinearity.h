#ifndef STATICNONLINEARITY_H
#define STATICNONLINEARITY_H

/* BeginDocumentation
 * Name: StaticNonLinearity
 *
 * Description: Static nonlinearity
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 * SeeAlso: module
 */

#include "../CImg-1.6.0_rolling141127/CImg.h"
#include <iostream>
#include "vector"
#include "module.h"

using namespace cimg_library;
using namespace std;

class StaticNonLinearity:public module{
protected:

    // type of nonlinearity
    int type;

    // nonlinearity parameters
    vector <double> slope;
    vector <double> offset;
    vector <double> exponent;
    vector <double> threshold;

    // for the piecewise function
    vector <double> start;
    vector <double> end;

    bool isThreshold;

    // buffers
    CImg<double> *inputImage;
    CImg<double> *outputImage;
    CImg<double> *markers;

public:
    // Constructor, copy, destructor.
    StaticNonLinearity(int x=1, int y=1, double temporal_step=1.0, int t=0);
    StaticNonLinearity(const StaticNonLinearity& copy);
    ~StaticNonLinearity(void);

    // Set functions
    void setSlope(double s=1.0, int segment=0);
    void setOffset(double o=0.0, int segment=0);
    void setExponent(double e=1.0, int segment=0);
    void setThreshold(double t=0.0, int segment=0);
    void setType(int t);

    // Allocate values
    virtual void allocateValues();
    virtual void setX(int x){sizeX=x;}
    virtual void setY(int y){sizeY=y;}
    // New input and update of equations
    virtual void feedInput(const CImg<double> &new_input, bool isCurrent, int port);
    virtual void update();
    // set Parameters
    virtual bool setParameters(vector<double> params, vector<string> paramID);
    virtual void clearParameters(vector<string> paramID);
    // Get output image (y(k))
    virtual CImg<double>* getOutput();
    // aux. func.
    template <typename T> int sgn(T val);
};

#endif // STATICNONLINEARITY_H
