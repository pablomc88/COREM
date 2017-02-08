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

#define LN_OUT_FILENAME_TAIL "_output_multimeter.txt"

// main
int main(int argc, char *argv[])
{
    string currentDirRoot = constants::getPath();

    // delete old LN analysis files in results folder (if any)
    DIR *dir;
    struct dirent *ent;
    
    string resdir = currentDirRoot+"results/";
    const char * charesdir = (resdir).c_str();

    if ((dir = opendir(charesdir)) != NULL) {
        size_t fn_tail_len;
        bool files_deleted=false;
        fn_tail_len = strlen(LN_OUT_FILENAME_TAIL);
        
        while((ent = readdir (dir)) != NULL) {
            size_t d_name_len;
            d_name_len=strlen(ent->d_name);
            
            if(d_name_len >= fn_tail_len && strcmp(ent->d_name+d_name_len-fn_tail_len, LN_OUT_FILENAME_TAIL)==0) {
                string removed_fn_path = resdir + ent->d_name;
                if(remove(removed_fn_path.c_str()) == -1)
                    perror("Error while deleting old multimeter files");
                files_deleted=true;                
            }
        
        }
        closedir (dir);
        if(files_deleted)
            cout << "Deleted previous *" LN_OUT_FILENAME_TAIL " files of ./results/ directory." << endl;
    }else{
        string tocreate = currentDirRoot+"results";
        cout << "Creating results/ directory" << endl;
        if(mkdir(tocreate.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
            perror("Error while creating results directory");
    }

    // Create retina interface
    string retinaString;
    int arg_index;
    bool got_script_file;
    bool verbose_flag, help_param, show_progress;

    // Default parameter values
    verbose_flag=false;
    show_progress=false;
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
        } else {
            if(strcmp(argv[arg_index],"-h") == 0 || strcmp(argv[arg_index],"--help") == 0){ // Help argument found
                cout << "COREM retina simulator." << endl;
                cout << " Syntax: " << argv[0] << " [-v] [-p] <retina_script_filename>" << endl;
                cout << "   <retina_script_filename> is a text file (usually with extension .py) which" << endl;
                cout << "   defines a retina model and simulation parameters." << endl;
                cout << "   -v argument shows verbose information." << endl;
                cout << "   -p argument shows progress information during simulation." << endl;
                cout << "   Visit https://github.com/pablomc88/COREM/wiki for information about the" << endl;
                cout << "   format of this script file" << endl;
                help_param=true;
            } else if(strcmp(argv[arg_index],"-v") == 0) // Verbose execution requested
                verbose_flag=true;
            else if(strcmp(argv[arg_index],"-p") == 0) // Progress information requested
                show_progress=true;
            else
                cout << "Ignoring unknown argument " << argv[arg_index] << endl;
        }
    }
    if(got_script_file){
        // Create interface
        int trial_ind, totalSimTime;
        double simStep, num_trials;
        const char *retinaSim = retinaString.c_str();

        // Simulation
        // Using a do loop we ensure that the InterfaceNEST is created at least one time, and
        // only one time if the number of trials is 1
        trial_ind=0;
        do {
            // Create new retina interface for every trial (reset values)
            InterfaceNEST interface;
            interface.setVerbosity(verbose_flag);
            if(!interface.allocateValues(retinaSim, LN_OUT_FILENAME_TAIL, constants::outputfactor, trial_ind)) {
                cout << "Incorrect parameter/value specified or resorce allocation. Aborting." << endl;
                break;
            }

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

            for(int sim_time=0;interface.getAbortExecution()==false && sim_time<totalSimTime;sim_time+=simStep){
                interface.update();
                if(show_progress)
                    cout << "\rSim. time: " << sim_time+simStep << " of: " << totalSimTime << "ms" << flush;
            }
            if(show_progress)
                cout << endl;
        } while(++trial_ind < num_trials); // Check the loop end condition in the end, after reading the number of trials
        
    }else{
        if(!help_param){
            cout << "Please provide a retina script filename in arguments" << endl;
            cout << "Execute '" << argv[0] << " -h' for more help" << endl;
        }
    }    
   return(!got_script_file); // Returns 0 when input retina script file is provided
}
