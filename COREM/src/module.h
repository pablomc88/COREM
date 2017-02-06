#ifndef MODULE_H
#define MODULE_H


/* BeginDocumentation
 * Name: module
 *
 * Description: base class of retina structure. GaussFilter, LinearFilter, ShortTermPlasticity,
 * SingleCompartment and StaticNonLinearity inherit from module
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 * SeeAlso: GaussFilter, LinearFilter, ShortTermPlasticity, SingleCompartment,
 * StaticNonLinearity
 */

#include <string>
#include <vector>
#include "../CImg-1.6.0_rolling141127/CImg.h"

using namespace cimg_library;
using namespace std;

class module{
protected:
    // Image size
    int sizeX, sizeY;
    // Length of the simulation step (in ms)
    double step;
    // Current simulation time (in ms)
    double simTime;
    // module ID
    string ID;

    // input modules and arithmetic operations for them
    vector <vector <int> > portArith;
    vector <vector <string> > modulesID;
    // types of input synapses
    vector<int> typeSynapse;

public:
    // Constructor, copy, destructor.
    module(int x=1,int y=1,double temporal_step=1.0);
    module(const module& copy);
    virtual ~module(void); // Destructor is declared virtual, so when a pointer to module class is deleted the derived class destructor is called

    // get/set protected parameters
    double getSizeX();
    double getSizeY();
    bool setSizeX(int x);
    bool setSizeY(int y);
    bool set_step(double temporal_step); // Set the duration of a simulation time step (slot) in milliseconds

    // add operations or ID of input modules
    void addOperation(vector <int> ops);
    void addID(vector <string> ID);
    // get operations or ID of input modules
    vector <int> getOperation(int op);
    vector <string> getID(int ID);
    // get size of vectors
    int getSizeID();
    int getSizeArith();

    // Set and get the name of the module
    void setModuleID(string s);
    string getModuleID();

    // Set and get synapse type
    void addTypeSynapse(int type);
    int getTypeSynapse(int port);

    bool checkID(const char* name);

    // virtual functions //
    // Allocate values
    virtual bool allocateValues();
    // New input and update of equations
    virtual void feedInput(double sim_time, const CImg<double>& new_input, bool isCurrent, int port);
    virtual void update();
    // Get output image (y(k))
    virtual CImg<double>* getOutput();
    // set Parameters
    virtual bool setParameters(vector<double> params, vector<string> paramID);
    virtual void clearParameters(vector<string> paramID);
    // This method can be called to find out if an object derived from class module performs
    // any usefull computation or it does not (and therefore can be deleted).
    // This method returns true if the object belongs to the base class (module). If the object
    // belongs to any derived class (such as SpikingOutput), it returns false.
    // Therefore this method is used to distingish objects from base class from those from a derived class.
    virtual bool isDummy();
};

#endif // MODULE_H

