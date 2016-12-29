#ifndef RETINA_H
#define RETINA_H

/* BeginDocumentation
 * Name: Retina
 *
 * Description: the class Retina implements a vector of modules, manages their connections
 * and feed them with new input data every simulation step.
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 * SeeAlso: module, GratingGenerator, fixationalMovGrating, impulse, whiteNoise
 */


#include "module.h"
#include "LinearFilter.h"
#include "SingleCompartment.h"
#include "GaussFilter.h"
#include "GratingGenerator.h"
#include "fixationalMovGrating.h"
#include "whiteNoise.h"
#include "impulse.h"
#include "SpikingOutput.h"
#include "SequenceOutput.h"
#include "StreamingInput.h"

using namespace cimg_library;
using namespace std;

class Retina{
protected:
    // Image size
    int sizeX, sizeY;
    // simulation step time length and ppd
    double step;
    double pixelsPerDegree;

    // Retina output and accumulator of intermediate images
    CImg <double> *output;
    CImg <double> *accumulator;
    // retina input channels (for color conversion)
    CImg<double> *RGBred, *RGBgreen, *RGBblue, *ch1, *ch2, *ch3, *rods, *X_mat, *Y_mat, *Z_mat;
    // vector of retina modules
    vector <module*> modules;
    // Type of input
    int inputType;

    // Inputs
    GratingGenerator *g;
    fixationalMovGrating *fg;
    whiteNoise *WN;
    impulse *imp;

    // total (end) simulation time and current simulation time
    int totalSimTime, simTime;
    // Current and total number of trials
    double CurrentTrial,totalNumberTrials;

    // Display comments
    bool verbose;

public:
    // Constructor, copy, destructor.
    Retina(int x=1,int y=1,double temporal_step=1.0);
    Retina(const Retina& copy);
    ~Retina(void);

    void reset(int x=1,int y=1,double temporal_step=1.0);

    // Allocate values and set protected parameters
    bool allocateValues();
    bool setSizeX(int x);
    bool setSizeY(int y);
    bool set_step(double temporal_step);
    int getSizeX();
    int getSizeY();
    double getStep();
    bool setVerbosity(bool verbose_flag);
    bool setSimCurrentTrial(double r);
    bool setSimTotalTrials(double r);
    bool setTotalSimTime(int t);
    double getSimCurrentTrial();
    double getSimTotalTrials();
    int getTotalSimTime();

    // set and get pixelsPerDegree
    bool setPixelsPerDegree(double ppd);
    double getPixelsPerDegree();
    // New input and update of equations
    CImg<double> *feedInput(int step);
    void update();

    // New module
    bool addModule(module* m, string ID);
    // Get module
    module* getModule(int ID);
    // get number of modules
    int getNumberModules();
    // Connect modules
    bool connect(vector <string> from, const char *to, vector <int> operations,const char *type_synapse);

    // Grating generator
    bool generateGrating(int type,double step,double lengthB,double length,double length2,int X,int Y,double freq,double T,double Lum,double Cont,double phi,double phi_t,double theta,double red, double green, double blue,double red_phi, double green_phi,double blue_phi);
    CImg<double> *updateGrating(double t);
    // Grating for fixational Movements
    bool generateFixationalMovGrating(int X,int Y,double radius,double jitter,double period,double step,double luminance,double contrast,double orientation,double red_weight,double green_weigh, double blue_weight, int type1, int type2, int ts);
    CImg<double> *updateFixGrating(double t);
    // White noise
    bool generateWhiteNoise(double mean, double contrast1,double contrast2, double period, double switchT,int X, int Y);
    CImg<double> *updateNoise(double t);
    whiteNoise* getWhiteNoise();
    // Impulse
    bool generateImpulse(double start, double stop, double amplitude, double offset, int X, int Y);
    CImg<double> *updateImpulse(double t);
    // Use streaming video or sequence as retina input
    // A valid (non-dummy) Input module must be inserted in the retina to use these inputs
    // We need this method to distingish the other retina input types from the others implemented as modules
    // Returns true on success.
    bool setModuleInput();
    // get number of images
    int getNumberImages();
};

#endif // RETINA_H
