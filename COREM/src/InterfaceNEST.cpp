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


int InterfaceNEST::getSimTime(){
    return totalSimTime;
}

//------------------------------------------------------------------------------//



double InterfaceNEST::getSimStep(){
    return step;
}

//------------------------------------------------------------------------------//

void InterfaceNEST::allocateValues(const char *retinaPath, const char * outputFile,double outputfactor,double currentRep){


    // Set input directory and parse the retina file
    FileReaderObject.setDir(retinaPath);
    FileReaderObject.allocateValues();
    FileReaderObject.parseFile(retina,displayMg);


    // Set simulation time to 0
    SimTime = 0;

    // Simulation parameters
    totalSimTime = retina.getSimTime();
    totalNumberTrials = retina.getSimTotalRep();

    CurrentTrial = currentRep;
    retina.setSimCurrentRep(currentRep);

    if(FileReaderObject.getContReading()){

        // retina size and step
        sizeX=retina.getSizeX();
        sizeY=retina.getSizeY();
        step=retina.getStep();

        // Display manager
        displayMg.setX(sizeX);
        displayMg.setY(sizeY);

        // LN parameters
        displayMg.setLNFile(outputFile,outputfactor);

        // Allocate retina object
        retina.allocateValues();


        // Display manager
        for(int k=1;k<retina.getNumberModules();k++){
            displayMg.addModule(k,(retina.getModule(k))->getModuleID());
        }


    }else
        abortExecution=true;


}



//------------------------------------------------------------------------------//


void InterfaceNEST::update(){


    CImg<double> *input= retina.feedInput(SimTime);
    retina.update();
    displayMg.updateDisplay(input,retina,SimTime,totalSimTime,CurrentTrial,totalNumberTrials);

    SimTime+=step;

}

//------------------------------------------------------------------------------//


double InterfaceNEST::getValue(double cell){

    int select_image = int(cell)/(sizeX*sizeY);
    int pos_im = int(cell) - select_image*(sizeX*sizeY);

    module* aux = retina.getModule(0);
    module* neuron;

    vector <string> layersID;
    layersID = aux->getID(select_image);

    const char * charIDO = (layersID[0]).c_str();
    int neuron_to_display = 0;

    for(int k=1;k<retina.getNumberModules();k++){
        neuron = retina.getModule(k);
        const char * charID = (neuron->getModuleID()).c_str();
        if (strcmp(charID,charIDO)==0){
            neuron_to_display = k;
            break;
        }
    }

    neuron = retina.getModule(neuron_to_display);

    int row = pos_im/sizeY;
    int col = pos_im%sizeY;

    return (*neuron->getOutput())(col,row,0,0);


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

