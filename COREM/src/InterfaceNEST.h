#ifndef INTERFACENEST_H
#define INTERFACENEST_H

/* BeginDocumentation
 * Name: InterfaceNEST
 *
 * Description: Interface with NEST. Functions update (integrate spatiotemporal equations)
 * and getValue (return membrane potential of neurons) allow communication with NEST.
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 * SeeAlso: DisplayManager, FileReader, GaussFilter, GratingGenerator, LinearFilter
 * module, Retina, ShortTermPlasticity, SingleCompartment, StaticNonLinearity
 */

#include "iostream"
#include "vector"

#include "DisplayManager.h"
#include "FileReader.h"
#include "Retina.h"

using namespace cimg_library;
using namespace std;

class InterfaceNEST{
protected:
    // Image size (sizeY = width, sizeX = height)
    int sizeX, sizeY;
    // simulation step
    double step;
    // Number of repetitions of each image (for a sequence)
    int repetitions;
    // Number of images in the input seq
    int numberImages;
    // Current and total simulation time
    int SimTime,totalSimTime;
    // Current and total number of trials
    double CurrentTrial,totalNumberTrials;

    // Retina
    Retina retina;
    // Displays
    DisplayManager displayMg;
    // FileReader
    FileReader FileReaderObject;

    bool abortExecution;

public:
    // Constructor, copy, destructor.
    InterfaceNEST(void);
    InterfaceNEST(const InterfaceNEST& copy);
    ~InterfaceNEST(void);

    double getTotalNumberTrials();
    int getSimTime();

    void reset(int X, int Y, double tstep,int rep);
    void allocateValues(const char * retinaPath, const char * outputFile, double outputfactor, double currentRep);
    void update();
    double getValue(double cell);
    bool getAbortExecution(){return abortExecution;}
    Retina& getRetina(){return retina;}
    double getSimStep();

    // modification of generators (for optimization)
    void setWhiteNoise(double mean, double contrast1,double contrast2, double period, double switchT,string id,double start, double stop);
    void setImpulse(double start, double stop, double amplitude,double offset);
    void setGrating(int type,double step,double lengthB,double length,double length2,int X,int Y,double freq,double T,double Lum,double Cont,double phi,double phi_t,double theta,double red, double green, double blue,double red_phi, double green_phi,double blue_phi);

};

#endif // INTERFACENEST_H
