#ifndef GRATINGGENERATOR_H
#define GRATINGGENERATOR_H

/* BeginDocumentation
 * Name: GratingGenerator
 *
 * Description: Sinusoidal grating generator
 *
 * Parameters:
 *
 * type -> 0=drifting. 1=static, oscillating. 2=static, reversing.
 * step -> time length of one frame, in seconds.
 * lengthB -> length of time before grating apparition, in seconds.
 * length -> length of first grating apparition.
 * length2 -> length of reversed grating apparition, in case of type=2.
 * X -> x_size of video, in pixels.
 * Y -> y_size of video, in pixels.
 * freq  -> temporal frequency, in hertz.
 * T  -> spatial period, in pixels.
 * Lum  -> mean luminance of the grating.
 * Cont  -> contrast of the grating. Between 0 and 1. If 1, then the image reaches luminance level 0.
 * phi -> spatial phase of central pixel at time zero, in multiples of Pi, for a COSINUS.
 * phi_t -> temporal phase of signal at time zero, in multiples of Pi, for a COSINUS. (only if type=1.)
 * theta -> theta. orientation of the grating.
 * r,g,b -> weight of each color channel.
 * red_phi,green_phi,blue_phi -> initial phase for each color channel.
 *
 * Source code adapted from Virtual Retina[1] (licensed under CeCILL-C)
 *
 * [1] Wohrer, Adrien, and Pierre Kornprobst. "Virtual Retina: a biological retina
 * model and simulator, with contrast gain control." Journal of computational
 * neuroscience 26.2 (2009): 219-249.
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *  SeeAlso: fixationalMovGrating, impulse, whiteNoise
 *
 */

#include "../CImg-1.6.0_rolling141127/CImg.h"
#include <iostream>

using namespace cimg_library;
using namespace std;

class GratingGenerator{
protected:

    // Grating parameters
    int type;
    double step;
    double lengthB;
    double length;
    double length2;
    int X;
    int Y;
    double freq;
    double T;
    double Lum;
    double Cont;
    double phi;
    double phi_t;
    double theta;
    double r,g,b;

    double Bsize;
    double first_grating_size;
    double second_grating_size;

    int x0;
    int y0;
    double cos_theta;
    double sin_theta;
    double A;

    double red_phi,green_phi,blue_phi;

    // Aux matrix
    CImg <double> aux;

public:
    // Constructor, copy, destructor.
    GratingGenerator();
    GratingGenerator(int ptype,double pstep,double plengthB,double plength,double plength2,int pX,int pY,double pfreq,double pT,double pLum,double pCont,double pphi,double pphi_t,double ptheta,double red, double green, double blue,double pred_phi, double pgreen_phi,double pblue_phi);
    GratingGenerator(const GratingGenerator& copy);
    ~GratingGenerator(void);

    // update the grating
    CImg<double> *compute_grating(double t);
};

#endif // GRATINGGENERATOR_H
