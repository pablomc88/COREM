
/* BeginDocumentation
 * Name: main
 *
 * Description: fixed time-step simulation of the retina script specified
 * in "constants.cpp" or passed through arguments.
 *
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 * SeeAlso:InterfaceNEST
 */

#include "../CImg-1.6.0_rolling141127/CImg.h"
#include "dirent.h"
#include "InterfaceNEST.h"
#include "constants.h"

using namespace cimg_library;
using namespace std;


// main
int main(int argc, char *argv[])
{

    string currentDirRoot = constants::getPath();

    // delete files in results folder (if any) (comment these lines when optimizing)
    DIR *dir;
    struct dirent *ent;
    string resdir = currentDirRoot+"results/";
    const char * charesdir = (resdir).c_str();

    if ((dir = opendir (charesdir)) != NULL) {
        int files = 0;
        while ((ent = readdir (dir)) != NULL) {
            files+=1;
          }

        if(files > 2){
            string results = "exec rm "+currentDirRoot+"results/*";
            const char * todelete = (results).c_str();
            system(todelete);
        }

    }closedir (dir);

    // Create retina interface
    string retinaString;
    const char * outID;

    // read arguments or default script
    if (argc==1){
        retinaString = currentDirRoot + constants::retinaScript;
        outID = constants::resultID;
    }else{
        retinaString = currentDirRoot + "Retina_scripts/" + (string)argv[1];
        if(argc==2)
            outID = argv[1];
        else if(argc==3)
            outID = argv[2];
    }

    const char * retinaSim = retinaString.c_str();
    InterfaceNEST interface;
    interface.allocateValues(retinaSim,outID,constants::outputfactor,0);

    // Read number of trials and simulation time
    double trials = interface.getTotalNumberTrials();
    int simTime = interface.getSimTime();
    double simStep = interface.getSimStep();

//    cout << "Simulation time: "<< simTime << endl;
//    cout << "Trials: "<< trials << endl;
//    cout << "Simulation step: "<< simStep << endl;

    // Simulation
    for(int i=0;i<trials;i++){

        // Create new retina interface for every trial (reset values)
        InterfaceNEST interface;
        interface.allocateValues(retinaSim,outID,constants::outputfactor,i);

//        cout << "-- Trial "<< i << " --" << endl;

        if(interface.getAbortExecution()==false){
            for(int k=0;k<simTime;k+=simStep){
                interface.update();
            }
        }

    }


   return 1;
}
