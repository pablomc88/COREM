#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include "Retina.h"

Retina::Retina(int x, int y, double temporal_step){
    step = temporal_step;
    sizeX=x;
    sizeY=y;
    pixelsPerDegree = 1.0;
    inputType = 0;
    numberImages = 0;
    repetitions = 0;

    verbose = false;

    modules.push_back((new module()));
    modules.back()->setModuleID("Output"); // Dummy output module used in case a particular output is not specified in script

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
    numberImages = copy.numberImages;
    repetitions = copy.repetitions;

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
    numberImages = 0;
    repetitions = 0;

    verbose = false;

    while(!modules.empty()) { // Destroy all the Retina modules and empty modules vector
        delete modules.back();
        modules.pop_back();
    }
    modules.push_back((new module()));
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

Retina& Retina::setSizeX(int x){
    if (x>0){
        sizeX = x;
    }
}

Retina& Retina::setSizeY(int y){
    if (y>0){
        sizeY = y;
    }
}

Retina& Retina::set_step(double temporal_step) {
    if (temporal_step>0){
        step = temporal_step;
    }
}

Retina& Retina::setPixelsPerDegree(double ppd){
    pixelsPerDegree = ppd;
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

void Retina::setVerbosity(bool verbose_flag){
    verbose = verbose_flag;
}

void Retina::setSimTotalRep(double r){
    totalNumberTrials = r;
}

void Retina::setSimCurrentRep(double r){
    CurrentTrial = r;
}

void Retina::setTotalSimTime(int t){
    totalSimTime = t;
}

double Retina::getSimCurrentRep(){
    return CurrentTrial;
}

double Retina::getSimTotalRep(){
    return totalNumberTrials;
}

int Retina::getTotalSimTime(){
    return totalSimTime;
}

//------------------------------------------------------------------------------//

void Retina::setRepetitions(int r){
    repetitions = r;
}

//------------------------------------------------------------------------------//


void Retina::allocateValues(){
    // Since Retina-class internal images are allocated in the constructor (and freed in the destructor)
    // they are not allocated here, just resized to match the last specified sizeX and sizeY
    if(verbose) {
        cout << "Allocating "<< (getNumberModules()-1) << " retinal modules." << endl;
        cout << "sizeX = "<< sizeX << endl;
        cout << "sizeY = "<< sizeY << endl;
        cout << "Temporal step = "<< step << " ms" << endl;
    }
    
    // Set current simulation time to 0 (this value is updated when feedInput() method is excuted)
    simTime = 0;

    // Realloc images according to current Retina size
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
    
    for (int i=0;i<modules.size();i++){
        module* m = modules[i];
        m->allocateValues();
    }
}


//------------------------------------------------------------------------------//

CImg<double> *Retina::feedInput(int step){
    CImg <double> *input;
    
    // Update Retina current simulation time from InterfaceNEST current simulation time
    simTime = step;

    // Input selection
    switch(inputType){
    case 0:
        if (step/repetitions < numberImages)
            input = inputSeq[step/repetitions];
        else
            input = inputSeq[numberImages-1];
        break;

    case 1:
        input = updateGrating(step);
        break;

    case 2:
        input = updateNoise(step);
        break;

    case 3:
        input = updateImpulse(step);
        break;

    case 4:
        input = updateFixGrating(step);
        break;

    default:
        cout << "Wrong input type!" << endl;
        break;
    }

    if (input->size()==sizeX*sizeY){
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

    for (int i=0;i<modules.size();i++){ // Feed the input of all modules (including Output module)

        module* neuron = modules[i];
        int number_of_ports = neuron->getSizeID();

        // port search
        for (int o=0;o<number_of_ports;o++){

            vector <string> l = neuron->getID(o);
            vector <int> p = neuron->getOperation(o);

            //image input
            const char * cellName = l[0].c_str();

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
            }else{

            // other inputs rather than cones or rods

                //search for the first image
                for (int m=1;m<modules.size();m++){ // Start from module 1: Do not consider output module as possible source
                    module* n = modules[m];
                    string cellName1 = l[0];
                    string cellName2 = n->getModuleID();
                    if (cellName1.compare(cellName2)==0){
                        *accumulator = *(n->getOutput());
                        break;
                    }
                }


                //other operations
                for (int k=1;k<l.size();k++){

                    for (int m=1;m<modules.size();m++){ // Start searching from module 1
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

            }

            if (neuron->getTypeSynapse(o)==0)
                neuron->feedInput(step, *accumulator, true, o);
            else
                neuron->feedInput(step, *accumulator, false, o);

        }
    }

    return input;
}


//------------------------------------------------------------------------------//

void Retina::update(){
    for (int i=0;i<modules.size();i++){ // Update all modules, including output module
        module* m = modules[i];
        m->update();
    }
}

//------------------------------------------------------------------------------//

bool Retina::addModule(module* new_module, string ID){
    bool correctly_added;

    new_module->setModuleID(ID);
    if(ID.compare("Output") == 0){ // The Output module is being added
        correctly_added=false; // Default return value
        // Search for the Output module in list of modules already added to the retina object
        for (int module_ind=0; module_ind < modules.size(); module_ind++){
            module *curr_module;
            curr_module = modules[module_ind];
            if (curr_module->checkID("Output")){ // Output module found
                // check if the Output module found is the expected default Output module (dummy)
                // Otherwise, output module has been already added and has been found 
                if(curr_module->isDummy()){ 
                    // Replace dummy output with Retina output module specified in the script file
                    delete curr_module; // Destroy dummy initial (should be module[0])
                    modules[module_ind] = new_module; // Use the specified module as Output module
                    correctly_added=true;
                } else {
                    cout << "Warning: Output module already added. Ignoring posterior ones" << endl;
                }
                break; // Exit for loop
            }
        }
    } else {
        modules.push_back(new_module);
        correctly_added=true;
    }
    if(verbose)cout << "Module "<< new_module->getModuleID() << " added to the retina structure. success: " << correctly_added << endl;
    
    return(correctly_added);
}

module* Retina::getModule(int ID){
    return modules[ID];
}

int Retina::getNumberModules(){
    return modules.size();
}

//------------------------------------------------------------------------------//


bool Retina::setInputSeq(string s){

    bool valueToReturn = false;
    struct stat input_seq_path_stat;
    
    // Check if the specified input-sequence path is a directory or a movie file
    if(stat(s.c_str(), &input_seq_path_stat) == 0){
        if(S_ISDIR(input_seq_path_stat.st_mode)){ // The user has specified a directory as input sequence: load all the directory files
            std::vector <std::string> result;
            DIR* dp=opendir(s.c_str());

            if (dp){
                dirent* de;
                do{ // For each directory entry:
                    de = readdir(dp);
                    if (de != NULL){ // We got a valid entry
                        std::string curr_input_file_path(s + de->d_name); // Compose the entire current dir entry path
                    
                        if(stat(curr_input_file_path.c_str(), &input_seq_path_stat) == 0){ // Try to get information about the de->d_name
                            if(!S_ISDIR(input_seq_path_stat.st_mode)){ // Directories (including . and ..) are not included in the input file sequence
                                result.push_back( curr_input_file_path );
                                valueToReturn = true; // At least one image was found, proceed
                            }
                        }else
                            perror("Error accessing the specified input sequence directory: ");
                    }
                }while (de != NULL); // Continue while we get valid directory entries
                closedir(dp);
            }else{
                cout << "Error reading retina script: Cannot open input sequence directory " << s << endl;
            }

            if (valueToReturn){
                CImg <double> image(result[0].c_str());  // First file of the sequence. result[1] and result[0] are current and up direcgory
                sizeX = image.height();
                sizeY = image.width();

                std::sort( result.begin(), result.end() );

                numberImages = result.size();
                inputSeq = new CImg<double>*[numberImages];

                for (int i=0;i<numberImages;i++){
                    inputSeq[i]=new CImg <double>(sizeY,sizeX,1,3);
                }

                for(int i=0;i<numberImages;i++){
                    CImg <double> image(result[i].c_str());
                    *(inputSeq[i])=image;
                }

                if(verbose) cout << numberImages << " images read from " << s << endl;
                inputType = 0;

              }
        } else { // The user has specified a file as input sequence: load all the movie file
        
            CImg <double> inp_movie(s.c_str()); // We assume that the specified file is a sequence of images
            sizeX = inp_movie.height();
            sizeY = inp_movie.width();
            numberImages = inp_movie.depth();

            inputSeq = new CImg<double>*[numberImages];

            for (int i=0;i<numberImages;i++){
                inputSeq[i]=new CImg <double>(sizeY,sizeX,1,3);
            }

            for(int i=0;i<numberImages;i++)
                *(inputSeq[i])=inp_movie.get_slice(i);

            if(verbose) cout << numberImages << " frames read from " << s << endl;
            inputType = 0;
            valueToReturn = true;
        }
    } else
        perror("Error accessing the specified input sequence: ");
    return valueToReturn;
}

//------------------------------------------------------------------------------//


int Retina::getNumberImages(){
    return numberImages;
}

//------------------------------------------------------------------------------//


bool Retina::connect(vector <string> from, const char *to,vector <int> operations,const char *type_synapse){
    bool valueToReturn = false;
    module* neuronto;

    // Search in the list of all modules (including Output module) for the specified target module
    for (int i=0;i<modules.size();i++){
        neuronto = modules[i];
        if (neuronto->checkID(to)){

            // check from
            for (int j=0;j< from.size();j++){
                int k;
                const char * ff = from[j].c_str();
                if (strcmp(ff,"rods")!=0 && strcmp(ff,"L_cones")!=0 && strcmp(ff,"M_cones")!=0 && strcmp(ff,"S_cones")!=0 && strcmp(ff,"red_channel")!=0 && strcmp(ff,"green_channel")!=0 && strcmp(ff,"blue_channel")!=0){
                    // Search in the list of all modules (excluding Output module) for the current module (ff) of the specified source module list (from)
                    for (k=1;k<modules.size();k++){
                        module* neuronfrom = modules[k];
                        if (neuronfrom->checkID(ff)){
                            valueToReturn = true;
                            if(verbose)cout << neuronfrom->getModuleID() << " has been conected to "<< neuronto->getModuleID() << endl;
                            break;
                        }

                    }
                    if (k==modules.size()){
                        valueToReturn=false;
                        break;
                    }
                }else{
                    valueToReturn = true; // Internal input type specified
                    break;
                }
            }


            if (valueToReturn){
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
            }
            break;
        }
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

