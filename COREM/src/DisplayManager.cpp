#include <cstddef> // for size_t type (used as loop index instead of int to avoid compile warnings)
#include "DisplayManager.h"

DisplayManager::DisplayManager(int x, int y){
    sizeX = x;
    sizeY = y;
    last_row=0;
    last_col=0;
    imagesPerRow=0;

    displayZoom = 0;
    delay = 0;
    imagesPerRow=4;

    numberModules = 0;
    valuesAllocated = false;

    // Indicate to destructor that these variables have not been allocated yet:
    intermediateImages = NULL;
    inputImage = NULL;
    templateBar = NULL;
    bars = NULL;
}

DisplayManager::DisplayManager(const DisplayManager& copy){
    cout << "Internal error: empty DisplayManager copy constructor has been called" << endl;
}

DisplayManager::~DisplayManager(void){
    // Free memory allocated in several parts of the class
    while(!displays.empty()) {
        delete displays.back();
        displays.pop_back();
    }
    
    while(!multimeters.empty()) {
        delete multimeters.back();
        multimeters.pop_back();
    }

    if(intermediateImages != NULL){
        for (int i=0;i<numberModules-1;i++)
            delete intermediateImages[i];
        delete[] intermediateImages;
    }

    if(bars != NULL){
        for (int i=0;i<numberModules-1;i++)
            delete bars[i];
        delete[] bars;
    }

    if(inputImage != NULL)
        delete inputImage;
        
    if(templateBar != NULL)
        delete templateBar;
}

void DisplayManager::setLNFile(const char *file, double ampl){
    LNFile = file;
    LNfactor = ampl;
}

void DisplayManager::reset(){
    sizeX = 1;
    sizeY = 1;
    last_row=0;
    last_col=0;
    imagesPerRow=0;

    displayZoom = 0;
    delay = 0;
    imagesPerRow=4;

    valuesAllocated = false;
}

//------------------------------------------------------------------------------//

bool DisplayManager::allocateValues(int number, double tstep){

    if(valuesAllocated == false){
        simStep = tstep;
        numberModules = number;

        // security check
        if (displayZoom*(double)sizeY >= CImgDisplay::screen_width()/4){
            displayZoom = CImgDisplay::screen_width()/(4.0*(double)sizeY);
            cout << "zoom has been readjusted to "<< displayZoom << endl;
        }

        for(int k=0;k<numberModules;k++){
            isShown.push_back(false);
        }

        for(int k=0;k<numberModules;k++){
            margin.push_back(0.0);
        }

        valuesAllocated = true;
    }
    return(true);
}

//------------------------------------------------------------------------------//


bool DisplayManager::setSizeX(int x){
    bool ret_correct;
    if (x>0){
        sizeX = x;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

bool DisplayManager::setSizeY(int y){
    bool ret_correct;    
    if (y>0){
        sizeY = y;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

void DisplayManager::setZoom(double zoom){
    displayZoom = zoom;
}

void DisplayManager::setDelay(int displayDelay){
    delay = displayDelay;
}

void DisplayManager::setImagesPerRow(int numberI){
    imagesPerRow = numberI;
}

void DisplayManager::setIsShown(bool value,int pos){
    isShown[pos]=value;
}

void DisplayManager::setMargin(double m, int pos){
    margin[pos]=m;
}


bool DisplayManager::setSimStep(double step_value){
    bool ret_correct;
    if(step_value > 0){
        simStep = step_value;
        cout << "Display simStep = "<< simStep << endl;
        ret_correct=true;
    } else
        ret_correct=false;
    return(ret_correct);
}

//------------------------------------------------------------------------------//


void DisplayManager::addMultimeterTempSpat(string multimeterID, string moduleID, int param1, int param2,bool temporalSpatial, string Show){

    multimeter* nm= new multimeter(sizeX,sizeY);
    nm->setSimStep(simStep);
    multimeters.push_back(nm);

    multimeterIDs.push_back(multimeterID);
    moduleIDs.push_back(moduleID);
    vector <int> aux;
    aux.push_back(param1);
    aux.push_back(param2);
    multimeterParam.push_back(aux);

    if(temporalSpatial)
        multimeterType.push_back(0);
    else
        multimeterType.push_back(1);

    const char * ShowChar = (Show).c_str();
    if(strcmp(ShowChar, "False") == 0)
        isShown.push_back(false);
    else
        isShown.push_back(true);

}

void DisplayManager::addMultimeterLN(string multimeterID, string moduleID, int x, int y, double segment, double interval, double start, double stop, string Show){

    multimeter* nm= new multimeter(sizeX,sizeY);
    nm->setSimStep(simStep);
    multimeters.push_back(nm);

    multimeterIDs.push_back(multimeterID);
    moduleIDs.push_back(moduleID);
    vector <int> aux;
    aux.push_back(x);
    aux.push_back(y);
    multimeterParam.push_back(aux);

    LNSegment.push_back(segment);
    LNInterval.push_back(interval);
    LNStart.push_back(start);
    LNStop.push_back(stop);

    multimeterType.push_back(2);

    const char * ShowChar = (Show).c_str();
    if(strcmp(ShowChar, "False") == 0)
        isShown.push_back(false);
    else
        isShown.push_back(true);
}

void DisplayManager::modifyLN(string moduleID, double start, double stop){

    size_t pos = 0;
    const char * str1 = moduleID.c_str();

    for(size_t k=0;k<multimeterIDs.size();k++){
        const char * str2 = (multimeterIDs[k]).c_str();
        if(strcmp(str1,str2)==0){
            pos = k;
            break;
        }
    }

    LNStart[pos] = start;
    LNStop[pos] = stop;
}

//------------------------------------------------------------------------------//

void DisplayManager::addModule(int pos,string ID){

    // Input
    if(displays.size() == 0){

        // black image
        double newX = (double)sizeX * displayZoom;
        double newY = (double)sizeY * displayZoom;
        CImg <double> image ((int)newY,(int)newX,1,1,0.0);

        // create input display
        if(isShown.size() > 0 && isShown[0]){
            CImgDisplay *input = new CImgDisplay(image,"Norm. input",0);
            input->move(0,0);
            displays.push_back(input);
            inputImage = new CImg <double>(sizeY,sizeX,1,1,0.0);
        }else{
            displays.push_back(new CImgDisplay());
        }

        // initialize intermediate images at the first call
        if(numberModules > 1){
            intermediateImages = new CImg<double>*[numberModules-1];
            for (int i=0;i<numberModules-1;i++)
              intermediateImages[i] = new CImg<double> (sizeY,sizeX,1,1,0.0);
        }
    }

    if(pos > 0 && isShown.size() > (size_t)pos) { // display for pos==0 (Input) is create above
        if(isShown[pos]){

            // black image
            double newX = (double)sizeX * displayZoom;
            double newY = (double)sizeY * displayZoom;
            CImg <double> image((int)newY,(int)newX,1,1,0.0);

            // Color Bar
            CImg <double> bar(50,(int)newX, 1, 1);
            cimg_forXY(bar,x,y) {
                bar(x,(int)newX-y-1,0,0)=255*((double)y/newX);
            }
            bar.map(CImg<double>::jet_LUT256());

            // Create window
            const char * WindowName = (ID).c_str();
            CImgDisplay *disp = new CImgDisplay((image,bar),WindowName,0);
            // Display
            (image,bar).display(*disp);

            // new row of the display
            int capacity = int((CImgDisplay::screen_width()-newY-100) / (newY+50));

            if (last_col<capacity && last_col < imagesPerRow){
                last_col++;
            }else{
                last_col = 1;
                last_row++;
            }

            // move display
            disp->move((int)last_col*(newY+80.0),(int)last_row*(newX+80.0));

            // Save display
            displays.push_back(disp);
        }else
            displays.push_back(new CImgDisplay());
    }
}

//------------------------------------------------------------------------------//


void DisplayManager::updateDisplay(CImg <double> *input, Retina &retina, int simTime, double totalSimTime, double numberTrials,double totalNumberTrials){

    double newX = (double)sizeX * displayZoom;
    double newY = (double)sizeY * displayZoom;

    double max=0.0,min=0.0;
    const unsigned char color[] = {255,255,255};


    // Display input
    if(isShown.size() > 0 && isShown[0] && input != NULL){

        CImgDisplay *d0 = displays[0];
        *inputImage = *input;

        inputImage->crop(margin[0],margin[0],0,0,sizeY-margin[0]-1,sizeX-margin[0]-1,0,inputImage->spectrum()-1,false);
        min = inputImage->min_max(max); // find maximum and minimum values in all color channels
        if(max-min > DBL_EPSILON) // normalize Input before showing it if it has different pixel values
            ((255*(*inputImage - min)/(max-min))).resize((int)newY,(int)newX).display(*d0);
        else // If we normalize we lose the offset and so all the informaton: we better don't normalize
            inputImage->resize((int)newY,(int)newX).display(*d0);
    }

    // Update windows
    if (numberModules>0 && simTime==0){
        bars = new CImg<double>*[numberModules-1];
        templateBar = new CImg <double>(50,(int)newX, 1, 1);
        for(int i=0;i<numberModules-1;i++){
            bars[i] = new CImg <double>(50,(int)newX, 1, 1);
        }
    }

    // copy interm. images
    for(int i=0;i<numberModules-1;i++){
        module* m = retina.getModule(i+1);
        CImg<double> *module_output = m->getOutput();
        if(module_output != NULL)
            *intermediateImages[i] = *module_output;
    }

    // show modules
    for(int k=0;k<numberModules-1;k++){
        if(isShown[k+1]){

            CImgDisplay *d = displays[k+1];

            // Color Bar
            *bars[k]=*templateBar;
            cimg_forXY(*(bars[k]),x,y) {
                (*bars[k])(x,(int)newX-y-1,0,0)=255*((double)y/newX);
            }
            bars[k]->map(CImg<double>::jet_LUT256());


            // find maximum and minimum values
            min = findMin(intermediateImages[k]);
            max = findMax(intermediateImages[k]);

            // draw them
            std::ostringstream strs1,strs2;

            if(abs(min)<100)
                strs1 << min;
            else
                strs1 << (int)min;

            string str1 = strs1.str();
            string min_value_text = str1.substr(0,4);

            bars[k]->draw_text(0,bars[k]->height()-20,min_value_text.c_str(),color,0,1,20);

            if(abs(max)<100)
                strs2 << max;
            else
                strs2 << (int)max;

            string str2 = strs2.str();
            string max_value_text = str2.substr(0,4);

            bars[k]->draw_text(0,10,max_value_text.c_str(),color,0,1,20);

            // show image
            intermediateImages[k]->crop(margin[k+1],margin[k+1],0,0,sizeY-margin[k+1]-1,sizeX-margin[k+1]-1,0,0,false);
            if(max-min > DBL_EPSILON) // normalize image before showing it if all its pixel do not the same value
                ((255*(*intermediateImages[k] - min)/(max-min)).map(CImg<double>::jet_LUT256()).resize((int)newY,(int)newX),*bars[k]).display(*d);
            else // Do not normalize to preserve the pixel offset information
                (intermediateImages[k]->map(CImg<double>::jet_LUT256()).resize((int)newY,(int)newX),*bars[k]).display(*d);
        }
    }

    if(input!=NULL) { // If the retina has input, update multimeters
        for(size_t i=0;i<multimeters.size();i++){
            multimeter *m = multimeters[i];
            module *n;
            const char * moduleID = (moduleIDs[i]).c_str();

            // find target module
            if(strcmp(moduleID, "Input") != 0){
                for(int j=1;j<retina.getNumberModules();j++){
                    n = retina.getModule(j);
                    if(n->checkID(moduleID)){
                        break;
                    }
                }
            }

            // temporal and LN mult.
            if(multimeterType[i]==0 || multimeterType[i]==2){
                vector <int> aux = multimeterParam[i];

                if(strcmp(moduleID, "Input") == 0){
                    m->recordValue((*input)(aux[0],aux[1],0,0));
                }else{
                    CImg<double> *module_output = n->getOutput();
                    if(module_output != NULL)
                        m->recordValue((*module_output)(aux[0],aux[1],0,0));
                }

                m->recordInput((*input)(aux[0],aux[1],0,0));
            }
            // spatial mult.
            else if(multimeterType[i]==1){
                vector <int> aux = multimeterParam[i];
                if(simTime >= aux[1] && simTime < aux[1]+simStep) { // aux[1] may not be divisible by simStep, we check that aux[1] is in the current sim. slot
                    // set position
                    int capacity = int((CImgDisplay::screen_width()-newY-100) / (newY+50));

                    if (last_col<capacity && last_col < imagesPerRow){
                        last_col++;
                    }else{
                        last_col = 1;
                        last_row++;
                    }

                    if(isShown[numberModules+i]==true){

                        if(strcmp(moduleID, "Input") == 0){

                            if(aux[0]>0)
                                m->showSpatialProfile(input,true,aux[0],multimeterIDs[i],(int)last_col*(newY+80.0),(int)last_row*(newX+80.0),-1);
                            else
                                m->showSpatialProfile(input,false,-aux[0],multimeterIDs[i],(int)last_col*(newY+80.0),(int)last_row*(newX+80.0),-1);
                        }else{
                            CImg<double> *module_output = n->getOutput();
                            if(module_output != NULL) {
                                if(aux[0]>0)
                                    m->showSpatialProfile(module_output,true,aux[0],multimeterIDs[i],(int)last_col*(newY+80.0),(int)last_row*(newX+80.0),-1);
                                else
                                    m->showSpatialProfile(module_output,false,-aux[0],multimeterIDs[i],(int)last_col*(newY+80.0),(int)last_row*(newX+80.0),-1);
                            }
                        }
                    }
                }
            }
        }
    }

    // display temporal and LN multimeters for the last simulation step
    if(valuesAllocated && (simTime==totalSimTime-simStep || input == NULL)) { // if time to show or end of input:
        int LNMultimeters = 0;
        for(size_t i=0;i<multimeters.size();i++){
            multimeter *m = multimeters[i];

            // set position
            if(multimeterType[i]==0 || multimeterType[i]==2){
                int capacity = int((CImgDisplay::screen_width()-newY-100) / (newY+50));

                if (last_col<capacity && last_col < imagesPerRow){
                    last_col++;
                }else{
                    last_col = 1;
                    last_row++;
                }
            }

            // temporal mult.
            if(multimeterType[i]==0){

                if(isShown[numberModules+i]==true){
                    if (i<multimeters.size()-1)
                        m->showTemporalProfile(multimeterIDs[i],(int)last_col*(newY+80.0),(int)last_row*(newX+80.0),delay,LNFile);
                    else
                        m->showTemporalProfile(multimeterIDs[i],(int)last_col*(newY+80.0),(int)last_row*(newX+80.0),-1,LNFile);

                }else{
                    m->showTemporalProfile(multimeterIDs[i],(int)last_col*(newY+80.0),(int)last_row*(newX+80.0),-2,LNFile);
                }

            // LN mult.
            }else if(multimeterType[i]==2){
                if(isShown[numberModules+i]==false){
                    m->showLNAnalysis(multimeterIDs[i],(int)last_col*(newY+80.0),(int)last_row*(newX+80.0),-2,LNSegment[LNMultimeters]/simStep,LNInterval[LNMultimeters]/simStep,LNStart[LNMultimeters]/simStep,LNStop[LNMultimeters]/simStep,totalNumberTrials,LNFile);
                }else{
                    if (i<multimeters.size()-1)
                        m->showLNAnalysis(multimeterIDs[i],(int)last_col*(newY+80.0),(int)last_row*(newX+80.0),delay,LNSegment[LNMultimeters]/simStep,LNInterval[LNMultimeters]/simStep,LNStart[LNMultimeters]/simStep,LNStop[LNMultimeters]/simStep,totalNumberTrials,LNFile);
                    else
                        m->showLNAnalysis(multimeterIDs[i],(int)last_col*(newY+80.0),(int)last_row*(newX+80.0),-1,LNSegment[LNMultimeters]/simStep,LNInterval[LNMultimeters]/simStep,LNStart[LNMultimeters]/simStep,LNStop[LNMultimeters]/simStep,totalNumberTrials,LNFile);
                }

                // check last trial to show average results
                if(numberTrials == totalNumberTrials-1){
                    m->getSwitchTime(retina.getWhiteNoise()->getSwitchTime());
                    if(isShown[numberModules+i]==false){
                        m->showLNAnalysisAvg((int)last_col*(newY+80.0),(int)last_row*(newX+80.0),-2,LNSegment[LNMultimeters]/simStep,LNStart[LNMultimeters]/simStep, LNStop[LNMultimeters]/simStep,totalNumberTrials,LNFile,LNfactor);
                    }else{
                        m->showLNAnalysisAvg((int)last_col*(newY+80.0),(int)last_row*(newX+80.0),0,LNSegment[LNMultimeters]/simStep,LNStart[LNMultimeters]/simStep, LNStop[LNMultimeters]/simStep,totalNumberTrials,LNFile,LNfactor);
                    }
                }
                LNMultimeters++;
            }
        }
    }
    // Show displays if there's an input display
    if(isShown.size() > 0 && isShown[0])
        displays[0]->wait(delay);
}

//------------------------------------------------------------------------------//


double DisplayManager::findMin(CImg<double> *input){
    double min = DBL_INF;
    cimg_forXY(*(input),x,y) {
        if ((*input)(x,y,0,0) < min)
            min =(*input)(x,y,0,0);
    }

    return min;
}

double DisplayManager::findMax(CImg<double> *input){
    double max = -DBL_INF;
    cimg_forXY(*(input),x,y) {
        if ((*input)(x,y,0,0) > max)
            max =(*input)(x,y,0,0);
    }

    return max;
}
