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

    valuesAllocated = false;

}

DisplayManager::DisplayManager(const DisplayManager& copy){

}

DisplayManager::~DisplayManager(void){

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

void DisplayManager::allocateValues(int number, double tstep){

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

}

//------------------------------------------------------------------------------//


void DisplayManager::setX(int x){
    sizeX = x;
}

void DisplayManager::setY(int y){
    sizeY = y;

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


void DisplayManager::setSimStep(double value){
    simStep = value;

    cout << "Display simStep = "<< simStep << endl;
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

    int pos = 0;
    const char * str1 = moduleID.c_str();

    for(int k=0;k<multimeterIDs.size();k++){
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
    if(displays.size()==0){

        // black image
        double newX = (double)sizeX * displayZoom;
        double newY = (double)sizeY * displayZoom;
        CImg <double> *image = new CImg <double>((int)newY,(int)newX,1,1,0.0);

        // create input display
        if(isShown[0]){
            CImgDisplay *input = new CImgDisplay(*image,"Input",0);
            input->move(0,0);
            displays.push_back(input);
            inputImage = new CImg <double>(sizeY,sizeX,1,1,0.0);
        }else{
            displays.push_back(new CImgDisplay());
        }

        // initialize intermediate images
        intermediateImages = new CImg<double>*[numberModules-1];
        for (int i=0;i<numberModules-1;i++){
          intermediateImages[i]=new CImg<double> (sizeY,sizeX,1,1,0.0);
        }

    }

    if(isShown[pos]){

        // black image
        double newX = (double)sizeX * displayZoom;
        double newY = (double)sizeY * displayZoom;
        CImg <double> *image = new CImg <double>((int)newY,(int)newX,1,1,0.0);

        // Color Bar
        CImg <double> *bar = new CImg <double>(50,(int)newX, 1, 1);
        cimg_forXY(*(bar),x,y) {
            (*bar)(x,(int)newX-y-1,0,0)=255*((double)y/newX);
        }
        bar->map(CImg<double>::jet_LUT256());

        // Create window
        const char * WindowName = (ID).c_str();
        CImgDisplay *disp = new CImgDisplay((*image,*bar),WindowName,0);
        // Display
        (*image,*bar).display(*disp);

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

    }else{
        displays.push_back(new CImgDisplay());
    }




}

//------------------------------------------------------------------------------//


void DisplayManager::updateDisplay(CImg <double> *input, Retina &retina, int step, double totalSimTime, double numberTrials,double totalNumberTrials){

    double newX = (double)sizeX * displayZoom;
    double newY = (double)sizeY * displayZoom;

    double max=0.0,min=0.0;
    const unsigned char color[] = {255,255,255};


    // Display input
    if(isShown[0]){

        CImgDisplay *d0 = displays[0];
        *inputImage = *input;

        // find maximum and minimum values
        min = findMin(inputImage);
        max = findMax(inputImage);

        inputImage->crop(margin[0],margin[0],0,0,sizeY-margin[0]-1,sizeX-margin[0]-1,0,0,false);
        (((255*(*inputImage - min)/(max-min))).resize((int)newY,(int)newX)).display(*d0);

    }


    // Update windows
    if (step==0){
        bars = new CImg<double>*[numberModules-1];
        templateBar = new CImg <double>(50,(int)newX, 1, 1);
        for(int i=0;i<numberModules-1;i++){
            bars[i] = new CImg <double>(50,(int)newX, 1, 1);
        }
    }


    //  copy interm. images
    for(int i=0;i<numberModules-1;i++){
        module* m = retina.getModule(i+1);
        *intermediateImages[i]= (*m->getOutput());
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
            const char * min_value_text = (str1.substr(0,4)).c_str();

            bars[k]->draw_text(0,bars[k]->height()-20,min_value_text,color,0,1,20);

            if(abs(max)<100)
                strs2 << max;
            else
                strs2 << (int)max;


            string str2 = strs2.str();
            const char * max_value_text = (str2.substr(0,4)).c_str();

            bars[k]->draw_text(0,10,max_value_text,color,0,1,20);

            // show image
            intermediateImages[k]->crop(margin[k+1],margin[k+1],0,0,sizeY-margin[k+1]-1,sizeX-margin[k+1]-1,0,0,false);
            (((255*(*intermediateImages[k] - min)/(max-min))).map(CImg<double>::jet_LUT256()).resize((int)newY,(int)newX),*bars[k]).display(*d);


        }
    }


    // update multimeters
    for(int i=0;i<multimeters.size();i++){
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
                m->recordValue((*n->getOutput())(aux[0],aux[1],0,0));
            }

            m->recordInput((*input)(aux[0],aux[1],0,0));
        }
        // spatial mult.
        else if(multimeterType[i]==1){
            vector <int> aux = multimeterParam[i];
            if(step == aux[1]){

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
                            m->showSpatialProfile((*input),true,aux[0],multimeterIDs[i],(int)last_col*(newY+80.0),(int)last_row*(newX+80.0),-1);
                        else
                            m->showSpatialProfile((*input),false,-aux[0],multimeterIDs[i],(int)last_col*(newY+80.0),(int)last_row*(newX+80.0),-1);

                    }else{
                        if(aux[0]>0)
                            m->showSpatialProfile((*n->getOutput()),true,aux[0],multimeterIDs[i],(int)last_col*(newY+80.0),(int)last_row*(newX+80.0),-1);
                        else
                            m->showSpatialProfile((*n->getOutput()),false,-aux[0],multimeterIDs[i],(int)last_col*(newY+80.0),(int)last_row*(newX+80.0),-1);

                    }
                }
            }
        }


    }


    // display temporal and LN multimeters for the last simulation step
    if(step==totalSimTime-simStep){

        int LNMultimeters = 0;
        for(int i=0;i<multimeters.size();i++){
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
        if(isShown[0])
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
