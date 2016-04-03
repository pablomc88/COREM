#ifndef IMPULSE_H
#define IMPULSE_H

/* BeginDocumentation
 * Name: impulse
 *
 * Description: impulse generator.
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 * SeeAlso: GratingGenerator, fixationalMovGrating, whiteNoise
 */

#include "../CImg-1.6.0_rolling141127/CImg.h"


using namespace cimg_library;
using namespace std;

class impulse{
private:
    // parameters
    double start;
    double stop;
    double amplitude;
    double offset;

    // Output image
    CImg <double> *output;

public:
    // Constructor, copy, destructor.
    impulse();
    impulse(double startParam, double stopParam, double amplitudeParam, double offsetParam,int X, int Y);
    impulse(const impulse& copy);
    ~impulse(void);

    // update
    CImg<double>* update(double t);
};

#endif // IMPULSE_H
