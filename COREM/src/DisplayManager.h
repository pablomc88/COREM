#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

/* BeginDocumentation
 * Name: DisplayManager
 *
 * Description: Displays of multimeters
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 * SeeAlso: multimeter
 */


#include "multimeter.h"
#include "Retina.h"

using namespace cimg_library;
using namespace std;

class DisplayManager{
protected:
    // Image size
    int sizeX, sizeY;

    // Display parameters
     double displayZoom;
     double delay;
     int imagesPerRow;
     vector <bool> isShown;
     vector <double> margin;

    // buffers of displays and images generated in each neural layer
    vector <CImgDisplay*> displays;
    CImg<double>** intermediateImages;

    // Multimeter buffer and parameters
    vector <multimeter*> multimeters;
    vector <string> multimeterIDs;
    vector <string> moduleIDs;
    vector <int> multimeterType;
    // to select either temporal or spatial multimeter
    vector < vector <int> > multimeterParam;
    // LN multimeters
    vector <double> LNSegment;
    vector <double> LNInterval;
    vector <double> LNStart;
    vector <double> LNStop;
    const char * LNFile;
    double LNfactor;

    // copy of input
    CImg <double> *inputImage;

    // last row to display
    int last_row,last_col;
    //color bars
    CImg <double> **bars;
    CImg <double> *templateBar;

    // number of modules
    int numberModules;

    // values allocated
    bool valuesAllocated;

    // Sim step
    double simStep;


public:
    // Constructor, copy, destructor.
    DisplayManager(int x=1,int y=1);
    DisplayManager(const DisplayManager& copy);
    ~DisplayManager(void);

    void reset();

    // Allocate values
    void allocateValues(int number,double tstep);

    // Set X and Y
    void setX(int x);
    void setY(int y);

    // other Set functions
    void setZoom(double zoom);
    void setDelay(int displayDelay);
    void setImagesPerRow(int numberI);
    void setIsShown(bool value, int pos);
    void setMargin(double m, int pos);

    // modify LN multimeter
    void modifyLN(string moduleID,double start,double stop);

    // Add display
    void addModule(int pos, string ID);

    // Add multimeter
    void addMultimeterTempSpat(string multimeterID, string moduleID, int param1, int param2, bool temporalSpatial, string Show);
    void addMultimeterLN(string multimeterID, string moduleID, int x, int y, double segment, double interval, double start, double stop, string Show);
    void setLNFile(const char * file, double ampl);

    // Update displays
    void updateDisplay(CImg <double> *input, Retina &retina, int step, double totalSimTime, double numberTrials,double totalNumberTrials);

    // Aux functions
    double findMin(CImg<double> *input);
    double findMax(CImg<double> *input);

    // Set Sim step
    void setSimStep(double value);


};

#endif // DISPLAYMANAGER_H
