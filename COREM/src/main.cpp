
/* BeginDocumentation
 * Name: main
 *
 * Description: fixed time-step simulation of the retina script
 * passed through arguments.
 *
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 * SeeAlso:InterfaceNEST
 */

#include <dirent.h>
#include "../CImg-1.6.0_rolling141127/CImg.h"
#include "InterfaceNEST.h"
#include "constants.h"

using namespace cimg_library;
using namespace std;


// main
int main(int argc, char *argv[])
{
    string currentDirRoot = constants::getPath();

    // delete files in results folder (if any)
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
            cout << "Deleting content of results/ directory" << endl;
            const char * todelete = (results).c_str();
            system(todelete);
        }
        closedir (dir);
    }else{
        string tocreate = "mkdir "+currentDirRoot+"results/";
        cout << "Creating results/ directory" << endl;
        system(tocreate.c_str());
    }

    // Create retina interface
    string retinaString;
    const char * outID;
    int arg_index;
    bool got_script_file;
    bool verbose_flag, help_param;

    // Default parameter values
    verbose_flag=false;
    help_param=false;
    got_script_file=false;
    // Parse all input arguments
    for(arg_index=1;arg_index<argc;arg_index++){
        if(argv[arg_index][0]!='-'){ // If first character of argument is not '-', assume that it is the script filename
            if(!got_script_file){
                retinaString = currentDirRoot + (string)argv[arg_index];
                got_script_file=true;
            }else
                cout << "Warning: More than one argument seem retina script filenames: Using the first one:" << retinaString << endl;
        }
        if(strcmp(argv[arg_index],"-h") == 0 || strcmp(argv[arg_index],"--help") == 0 || strcmp(argv[arg_index],"/?") == 0){ // Help argument found
            cout << "COREM retina simulator." << endl;
            cout << " Syntax: " << argv[0] << " [-v] <retina_script_filename>" << endl;
            cout << "   <retina_script_filename> is a text file (usually with extension .py) which" << endl;
            cout << "   defines a retina model and simulation parameters." << endl;
            cout << "   Visit https://github.com/pablomc88/COREM/wiki for information about the" << endl;
            cout << "   format of this script file" << endl;
            help_param=true;
        }
        
        if(strcmp(argv[arg_index],"-v") == 0) // Verbose execution requested
            verbose_flag=true;
    }
    if(got_script_file){
        // Create interface
        int trial_ind, totalSimTime;
        double simStep, num_trials;
        const char *retinaSim = retinaString.c_str();

        // Simulation
        // Using a do loop we ensure that the InterfaceNEST is created at least one time, and
        // only one time if the number of trails is 1
        trial_ind=0;
        do {
            // Create new retina interface for every trial (reset values)
            InterfaceNEST interface;
            interface.setVerbosity(verbose_flag);
            interface.allocateValues(retinaSim,"output",constants::outputfactor,trial_ind);

            if(trial_ind==0){ // Print info only in the first trial
                // Get number of trials and simulation time
                totalSimTime = interface.getTotalSimTime();
                simStep = interface.getSimStep();
                num_trials = interface.getTotalNumberTrials();

                if(verbose_flag){
                    cout << "Simulation time: " << totalSimTime << "ms" << endl;
                    cout << "Trials: "<< num_trials << endl;
                    cout << "Simulation step length: " << simStep << "ms" << endl;
                }
            }

            if(verbose_flag){
                cout << "-- Trial "<< trial_ind << " --" << endl;
                cout << "   AbortExecution " << interface.getAbortExecution() << endl;
            }

            if(interface.getAbortExecution()==false){
                for(int sim_time=0;sim_time<totalSimTime;sim_time+=simStep){
                    interface.update();
                }
            }

        } while(++trial_ind < num_trials); // Check the loop end condition in the end, after reading the number of trials
        
    }else{
        if(!help_param){
            cout << "Please provide a retina script filename in arguments" << endl;
            cout << "Execute '" << argv[0] << " -h' for more help" << endl;
        }
    }    
   return 1;
}
