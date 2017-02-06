#include <iostream>
#include <stdio.h>
#include "Retina.h"

Retina::Retina(int x, int y, double temporal_step){
    step = temporal_step;
    sizeX=x;
    sizeY=y;
    pixelsPerDegree = 1.0;
    inputType = -1; // Invalid retina input type

    verbose = false;

    // The fist element of modules (modules[0]) is a dummy Input module used in case a particular Input action is not
    // specified in the script (in this case if a new Input module is inserted the first one is replaced)
    modules.push_back(new module());
    modules.back()->setModuleID("Input");
    // The same for Output. InterfaceNEST expect that there is at least one Output module in the second
    // position (1) of vector modules.
    modules.push_back(new module());
    modules.back()->setModuleID("Output");

    output = new CImg <double>(sizeY, sizeX,1,1,0.0);
    accumulator = new CImg <double>(sizeY, sizeX,1,1,0.0);
    RGBred = new CImg <double>(sizeY, sizeX, 1, 1, 0.0);
    RGBgreen= new CImg <double>(sizeY, sizeX, 1, 1, 0.0);
    RGBblue= new CImg <double>(sizeY, sizeX, 1, 1, 0.0);
    ch1 = new CImg <double>(sizeY, sizeX, 1, 1, 0.0);
    ch2= new CImg <double>(sizeY, sizeX, 1, 1, 0.0);
    ch3= new CImg <double>(sizeY, sizeX, 1, 1, 0.0);
    rods= new CImg <double>(sizeY, sizeX, 1, 1, 0.0);
    X_mat= new CImg <double>(sizeY, sizeX, 1, 1, 0.0);
    Y_mat= new CImg <double>(sizeY, sizeX, 1, 1, 0.0);
    Z_mat= new CImg <double>(sizeY, sizeX, 1, 1, 0.0);
}

Retina::Retina(const Retina& copy){
    step = copy.step;
    sizeX= copy.sizeX;
    sizeY= copy.sizeY;
    pixelsPerDegree = copy.pixelsPerDegree;
    inputType = copy.inputType;
    verbose = copy.verbose;

    modules= copy.modules;

    output = new CImg <double>(*copy.output); // Member access operator (.) has more precedence than indirection (dereference) (*)
    accumulator = new CImg <double>(*copy.accumulator);
    RGBred = new CImg <double>(*copy.RGBred);
    RGBgreen= new CImg <double>(*copy.RGBgreen);
    RGBblue= new CImg <double>(*copy.RGBblue);
    ch1 = new CImg <double>(*copy.ch1);
    ch2= new CImg <double>(*copy.ch2);
    ch3= new CImg <double>(*copy.ch3);
    rods= new CImg <double>(*copy.rods);
    X_mat= new CImg <double>(*copy.X_mat);
    Y_mat= new CImg <double>(*copy.Y_mat);
    Z_mat= new CImg <double>(*copy.Z_mat);
}

Retina::~Retina(void){
    // We explicitly execute delete for objects created with new. Otherwise their object destructor is not called

    while(!modules.empty()) {
        delete modules.back();
        modules.pop_back();
    }
    
    delete output;
    delete accumulator;

    delete RGBred;
    delete RGBgreen;
    delete RGBblue;
    delete ch1;
    delete ch2;
    delete ch3;
    delete rods;
    delete X_mat;
    delete Y_mat;
    delete Z_mat;
}

void Retina::reset(int x,int y,double temporal_step){
    step = temporal_step;
    sizeX=x;
    sizeY=y;
    pixelsPerDegree = 1.0;
    inputType = 0;

    verbose = false;

    while(!modules.empty()) { // Destroy all the Retina modules and empty modules vector
        delete modules.back();
        modules.pop_back();
    }
    modules.push_back(new module());
    modules.back()->setModuleID("Input");
    modules.push_back(new module());
    modules.back()->setModuleID("Output");
    
    output->fill(0.0);
    accumulator->fill(0.0);

    RGBred->fill(0.0);
    RGBgreen->fill(0.0);
    RGBblue->fill(0.0);
    ch1->fill(0.0);
    ch2->fill(0.0);
    ch3->fill(0.0);
    rods->fill(0.0);
    X_mat->fill(0.0);
    Y_mat->fill(0.0);
    Z_mat->fill(0.0);
}

//------------------------------------------------------------------------------//

bool Retina::setSizeX(int x){
    bool ret_correct;    
    if (x>0){
        sizeX = x;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool Retina::setSizeY(int y){
    bool ret_correct;
    if (y>0){
        sizeY = y;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool Retina::set_step(double temporal_step) {
    bool ret_correct;
    if (temporal_step>0){
        step = temporal_step;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool Retina::setPixelsPerDegree(double ppd){
    bool ret_correct;
    if(ppd>0) {
        pixelsPerDegree = ppd;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

double Retina::getPixelsPerDegree(){
    return pixelsPerDegree;
}

int Retina::getSizeX(){
    return sizeX;
}

int Retina::getSizeY(){
    return sizeY;
}

double Retina::getStep(){
    return step;
}

bool Retina::setVerbosity(bool verbose_flag){
    verbose = verbose_flag;
    return(true);
}

bool Retina::setSimTotalTrials(double r){
    bool ret_correct;
    if(r >= 0) {
        totalNumberTrials = r;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool Retina::setSimCurrentTrial(double r){
    bool ret_correct;
    if(r >= 0) {
        CurrentTrial = r;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool Retina::setTotalSimTime(int t){
    bool ret_correct;
    if(t >= 0) {
        totalSimTime = t;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

double Retina::getSimCurrentTrial(){
    return CurrentTrial;
}

double Retina::getSimTotalTrials(){
    return totalNumberTrials;
}

int Retina::getTotalSimTime(){
    return totalSimTime;
}

//------------------------------------------------------------------------------//

bool Retina::allocateValues(){
    bool ret_correct;
    // The Input module (modules[0]) may want to adjust the image size, so we call allocateValues()
    // of this module first, and then propagate the new image size (normally the same) to the rest
    // of modules and retina
    modules[0]->setSizeX(sizeX);
    modules[0]->setSizeY(sizeY);
    modules[0]->allocateValues(); // Input module may determine a new size after allocateValues() call
    sizeX=modules[0]->getSizeX();
    sizeY=modules[0]->getSizeY();
    
    ret_correct = true;
    for (size_t i=1;i<modules.size();i++){ // For all modules except the Input one:
        module* m = modules[i];
        m->setSizeX(sizeX);
        m->setSizeY(sizeY);
        ret_correct = ret_correct && m->allocateValues();
    }

    if(verbose) {
        cout << "Allocating "<< (getNumberModules()-1) << " retinal modules." << endl;
        cout << "sizeX (height) = "<< sizeX << endl;
        cout << "sizeY (width)  = "<< sizeY << endl;
        cout << "Temporal step = "<< step << " ms" << endl;
    }
    
    // Set current simulation time to 0 (this value is updated when feedInput() method is excuted)
    simTime = 0;

    // Since Retina-class internal images are allocated in the constructor (and freed in the destructor)
    // they are not allocated here, just resized to match the last specified sizeX and sizeY
    output->assign(sizeY, sizeX, 1, 1, 0.0);
    accumulator->assign(sizeY, sizeX, 1, 1, 0.0);

    RGBred->assign(sizeY, sizeX, 1, 1, 0.0);
    RGBgreen->assign(sizeY, sizeX, 1, 1, 0.0);
    RGBblue->assign(sizeY, sizeX, 1, 1, 0.0);
    ch1->assign(sizeY, sizeX, 1, 1, 0.0);
    ch2->assign(sizeY, sizeX, 1, 1, 0.0);
    ch3->assign(sizeY, sizeX, 1, 1, 0.0);
    rods->assign(sizeY, sizeX, 1, 1, 0.0);
    X_mat->assign(sizeY, sizeX, 1, 1, 0.0);
    Y_mat->assign(sizeY, sizeX, 1, 1, 0.0);
    Z_mat->assign(sizeY, sizeX, 1, 1, 0.0);
    
    return(ret_correct);
}


//------------------------------------------------------------------------------//

CImg<double> *Retina::feedInput(int sim_time){
    CImg <double> *input;
    
    // Update Retina current simulation time from InterfaceNEST current simulation time
    simTime = sim_time;
    modules[0]->feedInput(sim_time, *accumulator, true, 0); // Inform input module about current simulation time (accumulator is not really used)
 
    // Input selection
    switch(inputType){

    case 0: // sequence input or streaming input
        input = modules[0]->getOutput(); // the output of input module does not change if update() is not called
        // The update of input (modules[0]->update()) is performed later, in Retina::update()
        break;

    case 1:
        input = updateGrating(sim_time);
        break;

    case 2:
        input = updateNoise(sim_time);
        break;

    case 3:
        input = updateImpulse(sim_time);
        break;

    case 4:
        input = updateFixGrating(sim_time);
        break;

    default:
        cout << "Wrong retina input type! Specify a correct retina input." << endl;
        input = NULL; // End simulation
        break;
    }

    if(input != NULL) { // We have input, so simulation can continue
        if(input->size() == (size_t)sizeX*(size_t)sizeY){
            // Separate color channels
            // cimg_forXY(img,x,y) is equivalent to cimg_forY(img,y) cimg_forX(img,x).
            // cimg_forX(img,x) is equivalent to for(int x=0;x<img.width();++x)
            cimg_forXY(*input,x,y) {
                (*RGBred)(x,y,0,0) = (*input)(x,y,0,0),    // Red component of image sent to imgR
                (*RGBgreen)(x,y,0,0) = (*input)(x,y,0,0),    // Green component of image sent to imgG
                (*RGBblue)(x,y,0,0) = (*input)(x,y,0,0);    // Blue component of image sent to imgB
            }
        }else{
           // Separate color channels
           cimg_forXY(*input,x,y) {
               (*RGBred)(x,y,0,0) = (*input)(x,y,0,0),    // Red component of image sent to imgR
               (*RGBgreen)(x,y,0,0) = (*input)(x,y,0,1),    // Green component of image sent to imgG
               (*RGBblue)(x,y,0,0) = (*input)(x,y,0,2);    // Blue component of image sent to imgB
           }
        }
        // Hunt-Pointer-Estévez (HPE) transform
        // sRGB --> XYZ
        *X_mat = 0.4124564*(*RGBblue) + 0.3575761*(*RGBgreen) + 0.1804375*(*RGBred);
        *Y_mat = 0.2126729*(*RGBblue) + 0.7151522*(*RGBgreen) + 0.0721750*(*RGBred);
        *Z_mat = 0.0193339*(*RGBblue) + 0.1191920*(*RGBgreen) + 0.9503041*(*RGBred);

        // XYZ --> LMS
        *ch1 = 0.38971*(*X_mat) + 0.68898*(*Y_mat) - 0.07868*(*Z_mat);
        *ch2 = -0.22981*(*X_mat) + 1.1834*(*Y_mat) + 0.04641*(*Z_mat);
        *ch3 = (*Z_mat);

        *rods = (*ch1 + *ch2 + *ch3)/3;

        for (size_t i=0;i<modules.size();i++){ // Feed the input of all modules (including Input module although it is not necessart)

            module* neuron = modules[i];

            // port search
            for (int o=0;o<neuron->getSizeID();o++){ // For all the module input connections:

                vector <string> l = neuron->getID(o);
                vector <int> p = neuron->getOperation(o);

                //image input
                const char * cellName = l[0].c_str(); // ID of the first port of current connection

                if(strcmp(cellName,"L_cones")==0){
                        *accumulator = *ch3;
                }else if(strcmp(cellName,"M_cones")==0){
                        *accumulator = *ch2;
                }else if(strcmp(cellName,"S_cones")==0){
                        *accumulator = *ch1;
                }else if(strcmp(cellName,"rods")==0){
                        *accumulator = *rods;
                // Inputs mainly used for testing
                }else if(strcmp(cellName,"red_channel")==0){
                        *accumulator = *RGBred;
                }else if(strcmp(cellName,"green_channel")==0){
                        *accumulator = *RGBgreen;
                }else if(strcmp(cellName,"blue_channel")==0){
                        *accumulator = *RGBblue;
                }else if(strcmp(cellName,"zeros")==0){
                        accumulator->fill(0.0);
                }else{

                // other inputs rather than cones or rods

                    //search for the first image
                    for (size_t m=0;m<modules.size();m++){ // Start from module 0: We consider Input module as possible source here although it it not necessary
                        module *n = modules[m];
                        string cellName1 = l[0];
                        string cellName2 = n->getModuleID();
                        if (cellName1.compare(cellName2)==0){
                            *accumulator = *(n->getOutput());
                            break;
                        }
                    }
                }

                // Accumulate input from other ports (perform other operations), even if the first port is a predefined input
                for (size_t k=1;k<l.size();k++){

                    for (size_t m=0;m<modules.size();m++){ // Search for source in all modules
                        module* n = modules[m];
                        string cellName1 = l[k];
                        string cellName2 = n->getModuleID();
                        if (cellName1.compare(cellName2)==0){

                           if (p[k-1]==0){
                                *accumulator += *(n->getOutput());
                            }else{
                                *accumulator -= *(n->getOutput());
                            }
                           break;
                        }
                    }
                }

                if (neuron->getTypeSynapse(o)==0)
                    neuron->feedInput(sim_time, *accumulator, true, o);
                else
                    neuron->feedInput(sim_time, *accumulator, false, o);
            }
        }
    }
    return input;
}


//------------------------------------------------------------------------------//

void Retina::update(){
    for (size_t i=0;i<modules.size();i++){ // Update all modules, including Output and Input modules
        module* m = modules[i];
        m->update();
    }
}

//------------------------------------------------------------------------------//

bool Retina::addModule(module *new_module, string new_mod_ID){
    bool correctly_added;
    const char *out_mod_id_start="Output"; // Output modules are recognied bececause their ID start with string
    const char *inp_mod_id="Input";
    bool out_mod_insertion; // An Output* module is being inserted

    new_module->setModuleID(new_mod_ID);
    // If the Input or an Output module is being added, first try to find the corresponding
    // initial dummy module and replace it with the currently being added module.
    // If no corresponding dummy module is found (Input/Output module was already inserted),
    // insert it anyway (if it is Output) or warn and ignore it (if is Input).
    out_mod_insertion = (new_mod_ID.compare(0,strlen(out_mod_id_start),out_mod_id_start) == 0); // An output module is being inserted
    if(new_mod_ID.compare(inp_mod_id) == 0 || out_mod_insertion){ // An input or output module is being inserted
        const char *dummy_mod_id;
        if(out_mod_insertion) // Output module is being inserted
            dummy_mod_id=out_mod_id_start; // Search for a dummy Input module
        else // Input module is being inserted
            dummy_mod_id=inp_mod_id; // Search for a dummy Output module
            
        correctly_added=false; // Default fn return value
        // Search for the Input module in list of modules already added to the retina object
        for(size_t module_ind=0; module_ind < modules.size() && !correctly_added; module_ind++){
            module *curr_module;
            curr_module = modules[module_ind];
            if (curr_module->checkID(dummy_mod_id)){ // Input/Output module found
                // check if the module found is the expected default module (dummy)
                // Otherwise, Input/Output module has already been added (and has been found)
                if(curr_module->isDummy()){ 
                    // Replace dummy module with new module specified (in the script file)
                    delete curr_module; // Destroy dummy initial module (should be module[0]:Input or module[1]:Output)
                    modules[module_ind] = new_module; // Use the specified module as Input module
                    correctly_added=true; // Exit for loop
                } else {
                    if(!out_mod_insertion) { // Other non-dummy Input module has been found
                        cout << "Warning: Retina Input has alredy been specified. Ignoring posterior one." << endl; // Ignore insertion (and return false)
                        break; // Exit for loop
                    } else { // We can have more than one Output module, so insert it anyway
                        modules.push_back(new_module);
                        correctly_added=true; // Exit for loop
                    }
                }
            }
        }
        if(!correctly_added && out_mod_insertion){ // All Output* modules must be inserted
            modules.push_back(new_module);
            correctly_added=true;            
        }
    } else { // For any other module, just insert it in the end of the modules vector
        modules.push_back(new_module);
        correctly_added=true;
    }
    if(verbose && correctly_added) cout << "Module "<< new_module->getModuleID() << " added to the retina." << endl;
    
    return(correctly_added);
}

module* Retina::getModule(int ID){
    return modules[ID];
}

int Retina::getNumberModules(){
    return modules.size();
}

//------------------------------------------------------------------------------//

bool Retina::setModuleInput(){
    inputType = 0; // Set retina input type=streaming or sequence
    return(true);
}

//------------------------------------------------------------------------------//


bool Retina::connect(vector <string> from, const char *to,vector <int> operations,const char *type_synapse){
    bool valueToReturn = false; // default return value
    module* neuronto;

    // Search in the list of all modules (excluding Input module (i=0)) for the specified target module
    for(size_t i=1;i<modules.size();i++){
        neuronto = modules[i];
        if(neuronto->checkID(to)){
            const char *ff=NULL;

            // connection target found. Let us search for all the sources (ports)
            // If any of them is not found, we set valueToReturn to false
            valueToReturn = true;
            // check that the modules in the 'from' list exist
            for(size_t j=0;j < from.size() && valueToReturn;j++){
                size_t k;

                ff = from[j].c_str();
                if(strcmp(ff,"rods")!=0 && strcmp(ff,"L_cones")!=0 && strcmp(ff,"M_cones")!=0 && strcmp(ff,"S_cones")!=0 && strcmp(ff,"red_channel")!=0 && strcmp(ff,"green_channel")!=0 && strcmp(ff,"blue_channel")!=0 && strcmp(ff,"zeros")!=0){ // Internal input type specified
                    // Search in the list of all modules (excluding the Intput module, which may return a multi-spectrum image) for the current module (ff) of the specified source module list (from)
                    for(k=1;k<modules.size();k++){
                        module* neuronfrom = modules[k];
                        if(neuronfrom->checkID(ff)) // Connection source found: exit the loop to make false the next if condition
                            break;
                    }
                    if(k==modules.size()) // Connection source not found
                        valueToReturn=false;
                }
            }

            if(valueToReturn){ // Shouldn't be inside the inner loop?
                neuronto->addID(from);
                neuronto->addOperation(operations);

                int typeSyn = 0;

                if(strcmp(type_synapse,"Current")==0){
                    typeSyn = 0;
                }else if(strcmp(type_synapse,"Conductance")==0){
                    typeSyn = 1;
                }else{
                    valueToReturn = false;
                }

                neuronto->addTypeSynapse(typeSyn);
                if(verbose) cout << from.size() << " sources (..." << ((ff!=NULL)?ff:"") << ") have been conected to " << neuronto->getModuleID() << " module." << endl;
            }
        } // May have more modules (Output modules) with the same ID, so continue looping
    }
    return valueToReturn;
}


//------------------------------------------------------------------------------//

bool Retina::generateGrating(int type,double step,double lengthB,double length,double length2,int X,int Y,double freq,double T,double Lum,double Cont,double phi,double phi_t,double theta,double red, double green, double blue,double red_phi, double green_phi,double blue_phi){

    bool valueToReturn = false;

    g = new GratingGenerator(type, step, lengthB, length, length2, X, Y, freq, T, Lum, Cont, phi, phi_t, theta,red,green,blue,red_phi, green_phi,blue_phi);
    sizeX = X;
    sizeY = Y;
    valueToReturn=true;

    inputType = 1;

    return valueToReturn;
}

CImg <double>* Retina::updateGrating(double t){
    return g->compute_grating(t);
}

//------------------------------------------------------------------------------//


bool Retina::generateWhiteNoise(double mean, double contrast1,double contrast2, double period, double switchT,int X, int Y){

    bool valueToReturn = false;

    WN = new whiteNoise(mean,contrast1,contrast2,period,switchT,X,Y);
    WN->initializeDist(CurrentTrial);

    sizeX = X;
    sizeY = Y;
    valueToReturn=true;

    inputType = 2;

    return valueToReturn;
}

CImg<double>* Retina::updateNoise(double t){
    return WN->update(t);
}

whiteNoise* Retina::getWhiteNoise(){
    return WN;
}


//------------------------------------------------------------------------------//


bool Retina::generateImpulse(double start, double stop, double amplitude,double offset, int X, int Y){

    bool valueToReturn = false;

    imp = new impulse(start,stop,amplitude,offset,X,Y);

    sizeX = X;
    sizeY = Y;
    valueToReturn=true;

    inputType = 3;

    return valueToReturn;
}

CImg<double>* Retina::updateImpulse(double t){
    return imp->update(t);
}

//------------------------------------------------------------------------------//

bool Retina::generateFixationalMovGrating(int X, int Y, double radius, double jitter, double period, double step, double luminance, double contrast, double orientation, double red_weight, double green_weigh, double blue_weight, int type1, int type2, int ts){

    bool valueToReturn = false;

    fg = new fixationalMovGrating(X,Y,radius, jitter, period, step, luminance, contrast, orientation, red_weight, green_weigh, blue_weight, type1, type2, ts);

    sizeX = X;
    sizeY = Y;
    valueToReturn=true;

    inputType = 4;

    return valueToReturn;
}

CImg <double>* Retina::updateFixGrating(double t){
    return fg->compute_grating(t);
}

