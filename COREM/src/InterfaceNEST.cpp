#include "InterfaceNEST.h"

InterfaceNEST::InterfaceNEST(void):retina(1,1,1.0),displayMg(1,1),FileReaderObject(1,1,1.0){
    abortExecution = false;
}

InterfaceNEST::InterfaceNEST(const InterfaceNEST& copy){
    abortExecution = false;

}

InterfaceNEST::~InterfaceNEST(void){

}


//------------------------------------------------------------------------------//


void InterfaceNEST::reset(int X, int Y, double tstep,int rep){
    abortExecution = false;
    FileReaderObject.reset(1,1,1.0);
    retina.reset(1,1,1.0);
    displayMg.reset();
}

//------------------------------------------------------------------------------//


double InterfaceNEST::getTotalNumberTrials(){
    return totalNumberTrials;
}

//------------------------------------------------------------------------------//


int InterfaceNEST::getTotalSimTime(){
    return totalSimTime;
}

//------------------------------------------------------------------------------//



double InterfaceNEST::getSimStep(){
    return step;
}

//------------------------------------------------------------------------------//

void  InterfaceNEST::setVerbosity(bool verbose_flag){
    retina.setVerbosity(verbose_flag);
}

bool InterfaceNEST::allocateValues(const char *retinaPath, const char * outputFile,double outputfactor,double currentRep){
    bool ret_correct;

    // Set input directory and parse the retina file
    FileReaderObject.setDir(retinaPath);
    FileReaderObject.allocateValues();
    FileReaderObject.parseFile(retina,displayMg);


    // Set simulation time to 0
    SimTime = 0;

    // Simulation parameters
    totalSimTime = retina.getTotalSimTime();
    totalNumberTrials = retina.getSimTotalTrials();

    CurrentTrial = currentRep;
    retina.setSimCurrentTrial(currentRep);

    if(FileReaderObject.getContReading()){

        // Allocate retina object
        ret_correct = retina.allocateValues();

        // retina size and step
        sizeX=retina.getSizeX();
        sizeY=retina.getSizeY();
        step=retina.getStep();

        // Display manager
        displayMg.setSizeX(sizeX);
        displayMg.setSizeY(sizeY);

        // LN parameters
        displayMg.setLNFile(outputFile,outputfactor);

        // Display manager
        for(int k=0;k<retina.getNumberModules();k++){ // we call addModule() even if retina ony has one module in order to always initialize Displays of Display Manager
            displayMg.addModule(k,(retina.getModule(k))->getModuleID());
        }

    }else {
        abortExecution=true;
        ret_correct=false;
    }
    return(ret_correct);
}



//------------------------------------------------------------------------------//


void InterfaceNEST::update(){
    CImg<double> *input;
    
    input = retina.feedInput(SimTime);
    retina.update(); // This call updates all the modules, so since input is a pointer the content may be modified
    displayMg.updateDisplay(input, retina, SimTime, totalSimTime, CurrentTrial, totalNumberTrials);
    SimTime+=step;
    if(input == NULL) // If this is the end of input, terminate simulation
        abortExecution=true;
}

//------------------------------------------------------------------------------//


double InterfaceNEST::getValue(double cell){

    int select_image = int(cell)/(sizeX*sizeY);
    int pos_im = int(cell) - select_image*(sizeX*sizeY);
    bool out_mod_found;
    module *out_mod;
    double neu_out_val;
    
    // Search for the Output module, which is used to generate input current for NEST neurons
    // This operation should be fast as the (first) Output module is the position 1 of the array
    // We start search at position 1, since in the first position is the Input module
    out_mod_found=false;
    for(int module_ind=1; module_ind < retina.getNumberModules() && !out_mod_found; module_ind++){
        out_mod = retina.getModule(module_ind);
        if(out_mod->checkID("Output"))
            out_mod_found=true; // Output module found: exit loop
    }

    if(out_mod_found && out_mod->getSizeID() > select_image){
        module *source_mod;
        vector <string> layersID;
        layersID = out_mod->getID(select_image);

        const char * charIDO = layersID[0].c_str();
        int neuron_to_display = 0;
        // Search for source module of Output module
        for(int k=0;k<retina.getNumberModules();k++){
            source_mod = retina.getModule(k);
            if(source_mod->checkID(charIDO)){
                neuron_to_display = k;
                break;
            }
        }
        source_mod = retina.getModule(neuron_to_display);

        int row = pos_im/sizeY;
        int col = pos_im%sizeY;
        neu_out_val = (*source_mod->getOutput())(col,row,0,0);
    } else
        neu_out_val = -1;

    return(neu_out_val);
}

//------------------------------------------------------------------------------//

bool InterfaceNEST::getAbortExecution(){
    return abortExecution;
}

//------------------------------------------------------------------------------//

Retina& InterfaceNEST::getRetina(){
    return retina;
}

//------------------------------------------------------------------------------//

void InterfaceNEST::setWhiteNoise(double mean, double contrast1, double contrast2, double period, double switchT,string id,double start, double stop){
    retina.generateWhiteNoise(mean, contrast1, contrast2, period,switchT,sizeX, sizeY);
    displayMg.modifyLN(id,start,stop);
}

void InterfaceNEST::setImpulse(double start, double stop, double amplitude,double offset){
    retina.generateImpulse(start, stop, amplitude,offset, sizeX, sizeY);
}

void InterfaceNEST::setGrating(int type,double step,double lengthB,double length,double length2,int X,int Y,double freq,double T,double Lum,double Cont,double phi,double phi_t,double theta,double red, double green, double blue,double red_phi, double green_phi,double blue_phi){
    retina.generateGrating(type, step, lengthB, length, length2, X, Y, freq, T, Lum, Cont, phi, phi_t, theta,red,green,blue,red_phi, green_phi,blue_phi);
}

