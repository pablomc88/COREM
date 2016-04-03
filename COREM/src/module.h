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

#include "../CImg-1.6.0_rolling141127/CImg.h"
#include "vector"
#include <iostream>

using namespace cimg_library;
using namespace std;

class module{
protected:
    // Image size
    int sizeX, sizeY;
    double step;
    // module ID
    string ID;

    // input modules and arithmetic operations among them
    vector <vector <int> > portArith;
    vector <vector <string> > modulesID;
    // types of input synapses
    vector<int> typeSynapse;

public:
    // Constructor, copy, destructor.
    module(int x=1,int y=1,double temporal_step=1.0);
    module(const module& copy);
    ~module(void);

    // set protected parameters
    module& setSizeX(int x);
    module& setSizeY(int y);
    module& set_step(double temporal_step);

    // add operations or ID of input modules
    void addOperation(vector <int> ops){portArith.push_back(ops);}
    void addID(vector <string> ID){modulesID.push_back(ID);}
    // get operations or ID of input modules
    vector <int> getOperation(int op){return portArith[op];}
    vector <string> getID(int ID){return modulesID[ID];}
    // get size of vectors
    int getSizeID(){return modulesID.size();}
    int getSizeArith(){return portArith.size();}

    // Set and get the name of the module
    void setModuleID(string s){ID=s;}
    string getModuleID(){return ID;}

    // Set and get synapse type
    void addTypeSynapse(int type){typeSynapse.push_back(type);}
    int getTypeSynapse(int port){return typeSynapse[port];}

    bool checkID(const char* name){
        const char * charID = ID.c_str();

        if (strcmp(name,charID) == 0){
            return true;
        }else{
            return false;
        }
    }

    // virtual functions //
    // Allocate values
    virtual void allocateValues(){}
    virtual void setX(int x){}
    virtual void setY(int y){}
    // New input and update of equations
    virtual void feedInput(const CImg<double>& new_input, bool isCurrent, int port){}
    virtual void update(){}
    // Get output image (y(k))
    virtual CImg<double>* getOutput(){}
    // set Parameters
    virtual bool setParameters(vector<double> params, vector<string> paramID){}
    virtual void clearParameters(vector<string> paramID){}
};

#endif // MODULE_H
