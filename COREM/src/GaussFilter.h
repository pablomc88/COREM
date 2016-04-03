#ifndef GAUSSFILTER_H
#define GAUSSFILTER_H

/* BeginDocumentation
 * Name: GaussFilter
 *
 * Description: Gaussian convolution to reproduce spatial integration in chemical
 * and gap-junction synapses. It implements a recursive infinite-impulse-response
 * (IIR) filter based on the Deriche's algorithm. Based on [1,2]. OpenMP is used
 * for multithreading
 *
 * [1] Triggs, Bill, and Michaël Sdika. "Boundary conditions for Young-van Vliet
 * recursive filtering." Signal Processing, IEEE Transactions on 54.6 (2006):
 * 2365-2367.
 *
 * [2] Tan, Sovira, Jason L. Dale, and Alan Johnston. "Performance of three
 * recursive algorithms for fast space-variant Gaussian filtering." Real-Time
 * Imaging 9.3 (2003): 215-228.
 *
 * Source code adapted from RawTherapee library. <http://rawtherapee.com/>
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 * SeeAlso:
 */

#include <omp.h>
#include "module.h"

using namespace std;
using namespace cimg_library;

class GaussFilter:public module{
protected:
    // Filter parameters
    double sigma;
    double* buffer;
    double pixelsPerDegree;

    // Deriche's coefficients
    double q,b0,b1,b2,b3,B;
    // Matrices
    double M[3][3];
    CImg <double> q_m,b0_m,b1_m,b2_m,b3_m,B_m,M_m;
    //spaceVariantSigma
    bool spaceVariantSigma;
    double R0,K;

    CImg<double> *inputImage;
    CImg<double> *outputImage;

public:

    // Constructor, copy, destructor.
    GaussFilter(int x=1,int y=1, double ppd=1.0);
    GaussFilter(const GaussFilter& copy);
    ~GaussFilter(void);

    // Allocate values and set protected parameters
    virtual void allocateValues();
    virtual void setX(int x){sizeX=x;}
    virtual void setY(int y){sizeY=y;}
    GaussFilter& setSigma(double sigm);

    // Fast filtering with constant sigma
    void gaussHorizontal(CImg<double> &src);
    void gaussVertical(CImg <double>& src);
    void gaussFiltering(CImg<double> &src);

    // Fast filtering with space-variant sigma
    void spaceVariantGaussHorizontal(CImg<double> &src);
    void spaceVariantGaussVertical(CImg<double> &src);
    void spaceVariantGaussFiltering(CImg<double> &src);
    double density(double r);

    // New input and update of equations
    virtual void feedInput(const CImg<double> &new_input, bool isCurrent, int port);
    virtual void update();
    // set Parameters
    virtual bool setParameters(vector<double> params, vector<string> paramID);
    // Get output image (y(k))
    virtual CImg<double>* getOutput();

};

#endif // GAUSSFILTER_H
