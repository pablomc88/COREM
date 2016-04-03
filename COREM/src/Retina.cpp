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

    modules.clear();
    modules.push_back((new module()));

    output = new CImg <double>(sizeY,sizeX,1,1,0.0);
    accumulator = *(new CImg <double>(sizeY,sizeX,1,1,0.0));

    RGBred = *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    RGBgreen= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    RGBblue= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    ch1 = *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    ch2= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    ch3= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    rods= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    X_mat= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    Y_mat= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    Z_mat= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));


}

Retina::Retina(const Retina& copy){

}

Retina::~Retina(void){

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

    modules.clear();
    modules.push_back((new module()));

    output = new CImg <double>(sizeY,sizeX,1,1,0.0);
    accumulator = *(new CImg <double>(sizeY,sizeX,1,1,0.0));

    RGBred = *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    RGBgreen= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    RGBblue= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    ch1 = *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    ch2= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    ch3= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    rods= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    X_mat= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    Y_mat= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    Z_mat= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
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

void Retina::setSimTotalRep(double r){
    totalNumberTrials = r;
}

void Retina::setSimCurrentRep(double r){
    CurrentTrial = r;
}

void Retina::setSimTime(int t){
    SimTime = t;
}

double Retina::getSimCurrentRep(){
    return CurrentTrial;
}

double Retina::getSimTotalRep(){
    return totalNumberTrials;
}

int Retina::getSimTime(){
    return SimTime;
}

//------------------------------------------------------------------------------//

void Retina::setRepetitions(int r){
    repetitions = r;
}

//------------------------------------------------------------------------------//


void Retina::allocateValues(){

    if(verbose)cout << "Allocating "<< (getNumberModules()-1) << " retinal modules." << endl;
    if(verbose)cout << "sizeX = "<< sizeX << endl;
    if(verbose)cout << "sizeY = "<< sizeY << endl;
    if(verbose)cout << "Temporal step = "<< step << " ms" << endl;
    output = new CImg <double>(sizeY,sizeX,1,1,0.0);
    accumulator = *(new CImg <double>(sizeY,sizeX,1,1,0.0));

    RGBred = *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    RGBgreen= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    RGBblue= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    ch1 = *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    ch2= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    ch3= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    rods= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    X_mat= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    Y_mat= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));
    Z_mat= *(new CImg <double>(sizeY,sizeX, 1, 1, 0.0));

    for (int i=1;i<modules.size();i++){
        module* m = modules[i];
        m->allocateValues();
    }
}


//------------------------------------------------------------------------------//

CImg<double> *Retina::feedInput(int step){
    CImg <double> *input;

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
        cimg_forXY(*input,x,y) {
            RGBred(x,y,0,0) = (*input)(x,y,0,0),    // Red component of image sent to imgR
            RGBgreen(x,y,0,0) = (*input)(x,y,0,0),    // Green component of image sent to imgG
            RGBblue(x,y,0,0) = (*input)(x,y,0,0);    // Blue component of image sent to imgB
        }
    }else{
       // Separate color channels
       cimg_forXY(*input,x,y) {
           RGBred(x,y,0,0) = (*input)(x,y,0,0),    // Red component of image sent to imgR
           RGBgreen(x,y,0,0) = (*input)(x,y,0,1),    // Green component of image sent to imgG
           RGBblue(x,y,0,0) = (*input)(x,y,0,2);    // Blue component of image sent to imgB
       }
    }
    // Hunt-Pointer-Estévez (HPE) transform
    // sRGB --> XYZ
    X_mat = 0.4124564*RGBblue + 0.3575761*RGBgreen + 0.1804375*RGBred;
    Y_mat = 0.2126729*RGBblue + 0.7151522*RGBgreen + 0.0721750*RGBred;
    Z_mat = 0.0193339*RGBblue + 0.1191920*RGBgreen + 0.9503041*RGBred;

    // XYZ --> LMS
    ch1 = 0.38971*X_mat + 0.68898*Y_mat - 0.07868*Z_mat;
    ch2 = -0.22981*X_mat + 1.1834*Y_mat + 0.04641*Z_mat;
    ch3 = Z_mat;

    rods = (ch1+ch2+ch3)/3;

for (int i=1;i<modules.size();i++){

    module* neuron = modules[i];
    int number_of_ports = neuron->getSizeID();

    // port search
    for (int o=0;o<number_of_ports;o++){

        vector <string> l = neuron->getID(o);
        vector <int> p = neuron->getOperation(o);

        //image input
        const char * cellName = l[0].c_str();

        if(strcmp(cellName,"L_cones")==0){
                accumulator=ch3;
        }else if(strcmp(cellName,"M_cones")==0){
                accumulator=ch2;
        }else if(strcmp(cellName,"S_cones")==0){
                accumulator=ch1;
        }else if(strcmp(cellName,"rods")==0){
                accumulator=rods;
        }else{

        // other inputs rather than cones or rods

            //search for the first image
            for (int m=1;m<modules.size();m++){
                module* n = modules[m];
                const char * cellName1 = l[0].c_str();
                const char * cellName2 = (n->getModuleID()).c_str();
                if (strcmp(cellName1,cellName2)==0){
                    accumulator = *(n->getOutput());
                    break;
                }
            }


            //other operations
            for (int k=1;k<l.size();k++){

                for (int m=1;m<modules.size();m++){
                    module* n = modules[m];
                    const char * cellName1 = l[k].c_str();
                    const char * cellName2 = (n->getModuleID()).c_str();
                    if (strcmp(cellName1,cellName2)==0){

                       if (p[k-1]==0){
                            accumulator += *(n->getOutput());
                        }else{
                            accumulator -= *(n->getOutput());
                        }
                       break;
                    }
                }

            }

    }

        if (neuron->getTypeSynapse(o)==0)
            neuron->feedInput(accumulator,true,o);
        else
            neuron->feedInput(accumulator,false,o);

    }
}



    return input;
}


//------------------------------------------------------------------------------//

void Retina::update(){
    for (int i=1;i<modules.size();i++){
        module* m = modules[i];
        m->update();
    }
}

//------------------------------------------------------------------------------//

void Retina::addModule(module* m, string ID){
    m->setModuleID(ID);
    modules.push_back(m);
    if(verbose)cout << "Module "<< m->getModuleID() << " added to the retina structure" << endl;
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
    const char * directory = s.c_str();

    std::vector <std::string> result;
    dirent* de;
    DIR* dp=opendir (directory);

    if (dp){
        while (true)
          {
              de = readdir( dp );
              if (de == NULL) break;
              result.push_back( std::string( de->d_name ) );
              std::sort( result.begin(), result.end() );
          }

        closedir( dp );
        valueToReturn = true;
    }

      if (valueToReturn){

          const char * ff = result[2].c_str();
          const char* dir = directory;
          char input_im_example[1000];
          strcpy(input_im_example,dir);
          strcat(input_im_example,ff);

          CImg <double> image(input_im_example);
          sizeX = image.height();
          sizeY = image.width();

          numberImages = result.size()-2;
          inputSeq = new CImg<double>*[numberImages];

          for (int i=0;i<numberImages;i++){
                   inputSeq[i]=new CImg <double>(sizeY,sizeX,1,3);
          }

          for(int i=0;i<numberImages;i++){
              const char * file = result[i+2].c_str();
              const char* dir = directory;
              char input_im[1000];
              strcpy(input_im,dir);
              strcat(input_im,file);

              CImg <double> image(input_im);
              *(inputSeq[i])=image;

          }

          if(verbose)cout << numberImages << " images read from "<< directory << endl;
          inputType = 0;

      }

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

            if (strcmp(to,"Output")==0){
                neuronto = modules[0];
                neuronto->addID(from);
                neuronto->addOperation(operations);
                valueToReturn = true;

                if(verbose)cout << from[0] << " has been added to the output buffer." << endl;

            }else{
                for (int i=1;i<modules.size();i++){
                    neuronto = modules[i];
                    if (neuronto->checkID(to)){

                        // check from
                        for (int j=0;j< from.size();j++){
                            int k;
                            const char * ff = from[j].c_str();
                            if (strcmp(ff,"rods")!=0 && strcmp(ff,"L_cones")!=0 && strcmp(ff,"M_cones")!=0 && strcmp(ff,"S_cones")!=0){
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
                                valueToReturn = true;
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

