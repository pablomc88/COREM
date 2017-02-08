#ifndef CONSTANTS_H
#define CONSTANTS_H

/* BeginDocumentation
 * Name: constants
 *
 * Description: declaration of constants used in the project and default path
 * to the retina script
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 */

#include <stdio.h>
#include <iostream>
#include <unistd.h>

// maximum path length
#define PATH_MAX 4096

// numerical constants
#define PI	M_PI
#define TWOPI	(2.0*PI)

#ifndef DBL_EPSILON
#define DBL_EPSILON 1.0e-9
#endif

#ifndef DBL_INF
#define DBL_INF 1.0e9
#endif

namespace constants{
    extern std::string retinaFolder;
    extern std::string retinaScript;
    extern const char* resultID;
    extern const double outputfactor;

    std::string getPath();

}


#endif // CONSTANTS_H
