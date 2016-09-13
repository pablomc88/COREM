#include "FileReader.h"

FileReader::FileReader(int X, int Y, double tstep){
    CorrectFile = true;
    continueReading = true;
    fileName = "";
}

void FileReader::reset(int X, int Y, double tstep){
    CorrectFile = true;
    continueReading = true;
    fileName = "";
}

FileReader::FileReader(const FileReader& copy){

}

FileReader::~FileReader(void){

}

//-------------------------------------------------//


void FileReader::setDir(const char *s){
    fileName = s;
}

//-------------------------------------------------//

void FileReader::allocateValues(){

    fin.open(fileName);

      if (!fin.good()) {
          cout << "Wrong retina file path." << endl;
          CorrectFile = false;
          continueReading = false;
      }
}

//-------------------------------------------------//

void FileReader::parseFile(Retina& retina, DisplayManager &displayMg){

    double verbose = false;
    int line = 0;
    int action = 0;

    while (!fin.eof() && CorrectFile && continueReading){

        line++;

        // read a line into memory
        char buf[MAX_CHARS_PER_LINE];
        fin.getline(buf, MAX_CHARS_PER_LINE);

        // parse the line into blank-delimited tokens
        int n = 0;

        // array to store memory addresses of the tokens in buf
        const char* token[MAX_TOKENS_PER_LINE] = {};

        // parse the line
        token[0] = strtok(buf, DELIMITER1); // first token
        bool change_delimiter = false;

        if (token[0]){ // zero if line is blank

            if (*token[0] != '#'){
                  for (n = 1; n < MAX_TOKENS_PER_LINE; n++)
                  {
                      if (change_delimiter == false){
                        token[n] = strtok(0, DELIMITER1); // subsequent tokens
                        if (!token[n]) break; // no more tokens
                        if (strcmp(token[n], "{") == 0)
                            change_delimiter = true;
                      }else{
                        token[n] = strtok(0, DELIMITER2);
                        if (!token[n]) break; // no more tokens
                      }

                  }
            }

        }

        if (continueReading && token[0]){

                if(strcmp(token[0], "retina") == 0)
                {
                    if(token[1]){
                    // Read action
                        if ( strcmp(token[1], "Create") == 0 ){
                            action = 1;
                        }
                        else if( strcmp(token[1], "Connect") == 0 ){                          
                            action = 2;
                        }
                        else if( strcmp(token[1], "TempStep") == 0 ){
                            action = 3;
                        }
                        else if( strcmp(token[1], "PixelsPerDegree") == 0 ){
                            action = 4;
                        }
                        else if( strcmp(token[1], "NRepetitions") == 0 ){
                            action = 5;
                        }
                        else if( strcmp(token[1], "DisplayDelay") == 0 ){
                            action = 6;
                        }
                        else if( strcmp(token[1], "DisplayWindows") == 0 ){
                            action = 7;
                        }
                        else if( strcmp(token[1], "Input") == 0 ){
                            action = 8;
                        }
                        else if( strcmp(token[1], "DensityScheme") == 0 ){
                            action = 9;
                        }
                        else if( strcmp(token[1], "SetDensity") == 0 ){
                            action = 10;
                        }
                        else if( strcmp(token[1], "Show") == 0 ){
                            action = 11;
                        }
                        else if( strcmp(token[1], "DisplayZoom") == 0 ){
                            action = 12;
                        }
                        else if( strcmp(token[1], "multimeter") == 0 ){
                            action = 13;
                        }
                        else if( strcmp(token[1], "SimTime") == 0 ){
                            action = 14;
                        }
                        else if( strcmp(token[1], "NumTrials") == 0 ){
                            action = 15;
                        }
                        else{
                            abort(line,"Unknown action command");
                            break;
                        }
                    }

                }
                else if (*token[0] != '#')
                {
                    abort(line,"neither 'retina' nor '#' token found at line beginning");
                    break;
                }


        }

        // retina configuration
        if(continueReading){
            switch(action){
            // Create
            case 1:

                if (token[2] && token[3]){
                    // read module type
                    module* newModule;
                    if (strcmp(token[2], "LinearFilter") == 0 ){
                        newModule = new LinearFilter(retina.getSizeX(),retina.getSizeY(),retina.getStep(),0.0);
                    }
                    else if (strcmp(token[2], "SingleCompartment") == 0 ){
                        newModule = new SingleCompartment(retina.getSizeX(),retina.getSizeY(),retina.getStep());
                    }
                    else if (strcmp(token[2], "StaticNonLinearity") == 0 ){
                        newModule = new StaticNonLinearity(retina.getSizeX(),retina.getSizeY(),retina.getStep(),0);
                    }
                    else if (strcmp(token[2], "CustomNonLinearity") == 0 ){
                        newModule = new StaticNonLinearity(retina.getSizeX(),retina.getSizeY(),retina.getStep(),1);
                    }
                    else if (strcmp(token[2], "SymmetricSigmoidNonLinearity") == 0 ){
                        newModule = new StaticNonLinearity(retina.getSizeX(),retina.getSizeY(),retina.getStep(),2);
                    }
                    else if (strcmp(token[2], "SigmoidNonLinearity") == 0 ){
                        newModule = new StaticNonLinearity(retina.getSizeX(),retina.getSizeY(),retina.getStep(),3);
                    }
                    else if (strcmp(token[2], "ShortTermPlasticity") == 0 ){
                        newModule = new ShortTermPlasticity(retina.getSizeX(),retina.getSizeY(),retina.getStep(),0.0,0.0,0.0,0.0,false);
                    }
                    else if (strcmp(token[2], "GaussFilter") == 0 ){
                        newModule = new GaussFilter(retina.getSizeX(),retina.getSizeY(),retina.getPixelsPerDegree());
                    }

                    else{
                        abort(line,"Unknown module type");
                        break;
                    }

                    // read parameters

                    if (token[4]){
                        if (strcmp(token[4], "{") == 0 && token[5]){

                            int i = 5;
                            vector<double> p;
                            vector<string> pid;

                            while(strcmp(token[i], "}") != 0 ){
                                if (token[i] && token[i+1]){

                                    if (strcmp(token[i], "type") == 0 || strcmp(token[i], "spaceVariantSigma") == 0){
                                        pid.push_back(token[i+1]);
                                        p.push_back(0.0);
                                    }else{
                                        pid.push_back(token[i]);
                                        p.push_back(atof(token[i+1]));
                                    }

                                    i+=2;
                                }else{
                                    abort(line,"Incorrect parameter format");
                                    break;
                                }
                                if (!token[i]){
                                    abort(line,"Incorrect format of parameters");
                                    break;
                                }
                            }


                            // Add module to the retina
                            if(continueReading){
                                continueReading=newModule->setParameters(p,pid);

                                if(continueReading){
                                    retina.addModule(newModule,token[3]);
                                }else{
                                    abort(line,"Error setting specified parameters");
                                    break;
                                }
                            }


                        }else{
                            abort(line,"Incorrect parameter start format");
                            break;
                        }
                    }else{
                        abort(line,"No parameter start found");
                        break;
                    }

                }

                action = 0;
                break;

            // Connect
            case 2:

                if (token[2] && token[3] && token[4]){

                    // Display Manager
                    displayMg.allocateValues(retina.getNumberModules(),retina.getStep());

                    vector <int> operations;
                    vector <string> modulesID;

                    // sequence of inputs
                    if (strcmp(token[2], "{") == 0 ){

                        int i = 3;
                        bool change = true;

                        while(strcmp(token[i], "}") != 0 ){
                            if (change){
                                modulesID.push_back(token[i]);
                                change=false;
                            }else{
                                if(strcmp(token[i],"+")==0){
                                    operations.push_back(0);
                                }else if(strcmp(token[i],"-")==0){
                                    operations.push_back(1);
                                }else
                                {
                                    abort(line,"Neither '+' or '-' token found");
                                    break;
                                }
                                change=true;
                            }
                            i+=1;
                            if (!token[i]){
                                abort(line,"Expected token not found");
                                break;
                            }
                        }

                        // Connect from modulesID to token[i+1]
                        if(token[i+1] && token[i+2] && continueReading){

                            continueReading=retina.connect(modulesID,token[i+1],operations,token[i+2]);
                            if(verbose)cout << "Modules connected to "<< token[i+1] << endl;

                            if (!continueReading){
                                abort(line,"Error connecting expecified modules");
                                break;
                            }

                        }else{
                            abort(line,"Expected two module IDs and correct format");
                            break;
                        }

                    }
                    // Only one input
                    else{
                        modulesID.push_back(token[2]);
                        continueReading=retina.connect(modulesID,token[3],operations,token[4]);
                        if(verbose)cout << "Modules connected to "<< token[3] << endl;
                        if (!continueReading){
                            abort(line,"Incorrect module IDs for connect action command");
                            break;
                        }
                    }
                }else{
                    abort(line,"Not enough parameters");
                    break;
                }

                action = 0;
                break;
            // Temporal step
            case 3:

                if (token[2]){
                    if (atof(token[2])>0)
                        retina.set_step(atof(token[2]));
                    else{
                        abort(line,"Expected positive value (>0)");
                        break;
                    }
                }else{
                    abort(line,"Expected temporal step value");
                    break;
                }

                if(verbose)cout << "Temporal step = "<< atof(token[2]) << endl;
                action = 0;
                break;
            // Pixels per degree
            case 4:
                if (token[2] && token[3] && token[4]){

                    if (strcmp(token[2], "{") == 0 && strcmp(token[4], "}") == 0){
                        if (atof(token[3])>0.0){
                            retina.setPixelsPerDegree(atof(token[3]));
                            if(verbose)cout << "Pixels per degree = " << atof(token[3]) << endl;
                        }
                        else{
                            abort(line,"Expected positive parameter (>0)");
                            break;
                        }
                    }else{
                        abort(line,"Expected '{' and '}' around parameter");
                        break;
                    }


                }else{
                    abort(line,"Expected pixel per degree parameter");
                    break;
                }

                action = 0;
                break;

            // Number of repetitions
            case 5:

                if (token[2]){
                    if (atof(token[2])>0)
                        retina.setRepetitions(atof(token[2]));
                    else{
                        abort(line,"Expected a positive number of repetitions (>0)");
                        break;
                    }
                }else{
                    abort(line,"Expected value of number of repetitions");
                    break;
                }

                if(verbose)cout << "Number of repetitions = "<< atof(token[2]) << endl;
                action = 0;
                break;
            // Display delay
            case 6:

                if (token[2]){
                    if (atof(token[2])>=0)
                        displayMg.setDelay(atof(token[2]));
                    else{
                        abort(line,"Expected positive or zero value (>=0)");
                        break;
                    }
                }else{
                    abort(line,"Expected value of display delay");
                    break;
                }

                if(verbose)cout << "Display delay = "<< atof(token[2]) << endl;
                action = 0;
                break;

            // Windows
            case 7:

                if (token[2]){
                    if (atof(token[2])>0)
                        displayMg.setImagesPerRow(atof(token[2]));
                    else{
                        abort(line,"Expected a positive value (>0)");
                        break;
                    }
                }else{
                    abort(line,"Expected number of windows");
                    break;
                }

                if(verbose)cout << "Windows configured." << endl;
                action = 0;
                break;
            // Input
            case 8:

                if (token[2] && token[3] && token[4] && token[5]){
                    if (strcmp(token[3], "{") == 0 ){
                        // Input sequence
                        if (strcmp(token[2], "sequence") == 0 ){
                            continueReading=retina.setInputSeq(token[4]);
                            if(verbose)cout << "Input sequence read." << endl;

                        }else if(strcmp(token[2], "grating") == 0 ){

                            if (strcmp(token[4],"type")==0 && strcmp(token[6],"step")==0 && strcmp(token[8],"length1")==0 && strcmp(token[10],"length2")==0 && strcmp(token[12],"length3")==0 && strcmp(token[14],"sizeX")==0 && strcmp(token[16],"sizeY")==0 && strcmp(token[18],"freq")==0 && strcmp(token[20],"period")==0 && strcmp(token[22],"Lum")==0 && strcmp(token[24],"Contr")==0 && strcmp(token[26],"phi_s")==0 && strcmp(token[28],"phi_t")==0 && strcmp(token[30],"orientation")==0 && strcmp(token[32],"red_weight")==0 && strcmp(token[34],"green_weight")==0 && strcmp(token[36],"blue_weight")==0 && strcmp(token[38],"red_phase")==0 && strcmp(token[40],"green_phase")==0 && strcmp(token[42],"blue_phase")==0 && strcmp(token[44],"}")==0){
                                continueReading=retina.generateGrating(atof(token[5]),atof(token[7]),atof(token[9]),atof(token[11]),atof(token[13]),atof(token[15]),atof(token[17]),atof(token[19]),atof(token[21]),atof(token[23]),atof(token[25]),atof(token[27]),atof(token[29]),atof(token[31]),atof(token[33]),atof(token[35]),atof(token[37]),atof(token[39]),atof(token[41]),atof(token[43]));
                                retina.setRepetitions(1.0);
                                if(verbose)cout << "Grating generated." << endl;
                            }else{
                                abort(line,"Expected parameter list of grating: 'type','step','length1','length2','length3','sizeX','sizeY','freq','period','Lum','Contr','phi_s','phi_t','orientation','red_weight','green_weight','blue_weight','red_phase','green_phase','blue_phase'");
                                break;
                            }
                        }else if(strcmp(token[2], "fixationalMovGrating") == 0 ){

                            if (strcmp(token[4],"sizeX")==0 && strcmp(token[6],"sizeY")==0 && strcmp(token[8],"circle_radius")==0 && strcmp(token[10],"jitter_period")==0 && strcmp(token[12],"spatial_period")==0 && strcmp(token[14],"step_size")==0 && strcmp(token[16],"Lum")==0 && strcmp(token[18],"Contr")==0 && strcmp(token[20],"orientation")==0 && strcmp(token[22],"red_weight")==0 && strcmp(token[24],"green_weight")==0 && strcmp(token[26],"blue_weight")==0 && strcmp(token[28],"type1")==0 && strcmp(token[30],"type2")==0 && strcmp(token[32],"switch")==0 && strcmp(token[34],"}")==0){
                                continueReading=retina.generateFixationalMovGrating(atof(token[5]),atof(token[7]),atof(token[9]),atof(token[11]),atof(token[13]),atof(token[15]),atof(token[17]),atof(token[19]),atof(token[21]),atof(token[23]),atof(token[25]),atof(token[27]),atof(token[29]),atof(token[31]),atof(token[33]));
                                retina.setRepetitions(1.0);
                                if(verbose)cout << "Grating of fixational movements generated." << endl;
                            }else{
                                abort(line,"Expected parameter list of fixationalMovGrating: 'sizeX','sizeY','circle_radius','jitter_period','spatial_period','step_size','Lum','Contr','orientation','red_weight','green_weight','blue_weight','type1','type2','switch'");
                                break;
                            }

                        }else if(strcmp(token[2], "whiteNoise") == 0 ){
                            if (strcmp(token[4],"mean")==0 && strcmp(token[6],"contrast1")==0 && strcmp(token[8],"contrast2")==0 && strcmp(token[10],"period")==0 && strcmp(token[12],"switch")==0 && strcmp(token[14],"sizeX")==0 && strcmp(token[16],"sizeY")==0 && strcmp(token[18],"}")==0){
                                continueReading=retina.generateWhiteNoise(atof(token[5]),atof(token[7]),atof(token[9]),atof(token[11]),atof(token[13]),atof(token[15]),atof(token[17]));
                                retina.setRepetitions(1.0);
                                if(verbose)cout << "White noise generated." << endl;
                            }else{
                                abort(line,"Expected parameter list of whiteNoise: 'mean','contrast1','contrast2','period','switch','sizeX','sizeY'");
                                break;
                            }
                        }
                        else if(strcmp(token[2], "impulse") == 0 ){
                            if (strcmp(token[4],"start")==0 && strcmp(token[6],"stop")==0 && strcmp(token[8],"amplitude")==0 && strcmp(token[10],"offset")==0 && strcmp(token[12],"sizeX")==0 && strcmp(token[14],"sizeY")==0 && strcmp(token[16],"}")==0){
                                continueReading=retina.generateImpulse(atof(token[5]),atof(token[7]),atof(token[9]),atof(token[11]),atof(token[13]),atof(token[15]));
                                retina.setRepetitions(1.0);
                                if(verbose)cout << "Impulse generated." << endl;
                            }else{
                                abort(line,"Expected parameter list of impulse: 'start','stop','amplitude','offset','sizeX','sizeY'");
                                break;
                            }
                        }
                        else{
                            abort(line,"Unknown input type");
                            break;
                        }


                    }
                    else{
                        abort(line,"Parameter start token ('{') not found");
                        break;
                    }
                }else{
                    abort(line,"Tokens for input action command not found");
                    break;
                }

                action = 0;
                break;

            // Density scheme
            case 9:
                if(verbose)cout << "New Density scheme created." << endl;
                action = 0;
                break;
            // Density assignment
            case 10:
                if(verbose)cout << "Density assigned to modules." << endl;
                action = 0;
                break;
            // Show windows
            case 11:

                if (token[2] && token[3] && token[4] && token[5]){
                    bool module_id_found;
                    // Search for the module ID
                    module_id_found=false;
                    continueReading=false;
                    for(int l=1;l<retina.getNumberModules();l++){
                        module *m = retina.getModule(l);
                        string ID = m->getModuleID();
                        if(ID.compare(token[2])==0){
                            continueReading=true;
                            module_id_found=true;
                            if(strcmp(token[3],"True")==0){
                                displayMg.setIsShown(true,l);
                                if(verbose)cout << "Module "<<  token[2] <<" is displayed." << endl;
                                // also input
                                displayMg.setIsShown(true,0);
                            }
                            else if(strcmp(token[3],"False")==0){
                                displayMg.setIsShown(false,l);
                                if(verbose)cout << "Module "<<  token[2] <<" is not displayed." << endl;
                            }
                            else{
                                abort(line,"Expected a 'True' or 'False' value after module ID");
                                break;
                            }

                            if(atof(token[5])>=0){
                                displayMg.setMargin(atof(token[5]),l);
                            }

                        }
                    }
                    // Check for input
                    if(strcmp(token[2],"Input")==0){
                            continueReading=true;
                            if(strcmp(token[3],"True")==0){
                                displayMg.setIsShown(true,0);
                                if(verbose)cout << "Module "<<  token[2] <<" is displayed." << endl;
                            }
                            else if(strcmp(token[3],"False")==0){
                                displayMg.setIsShown(false,0);
                                if(verbose)cout << "Module "<<  token[2] <<" is not displayed." << endl;
                            }
                            else{
                                abort(line,"Expected 'True' or 'False' value for Input");
                                break;
                            }

                            if(atof(token[5])>=0){
                                displayMg.setMargin(atof(token[5]),0);
                            }

                    }
                    else
                        if(!module_id_found)
                            abort(line,"Specified module ID not found");

                }else{
                    abort(line,"Parameters for Show window not found");
                    break;
                }

                action = 0;
                break;

            // Window Zoom
            case 12:

                if (token[2] && token[3] && token[4]){

                    if (strcmp(token[2], "{") == 0 && strcmp(token[4], "}") == 0){
                        if (atof(token[3])>0.0){
                            displayMg.setZoom(atof(token[3]));
                            if(verbose)cout << "Display zoom = " << atof(token[3]) << endl;
                        }
                        else{
                            abort(line,"Expected a positive value (>0)");
                            break;
                        }
                    }else{
                        abort(line,"Expected '{' and '}' around parameter value");
                        break;
                    }


                }else{
                    abort(line,"Expected parameters for Windows Zoom not found");
                    break;
                }

                action = 0;
                break;

            // multimeter
            case 13:

                if (token[2] && token[3] && token[4] && token[5]){
                    // read multimeter type
                    if (strcmp(token[2], "spatial") != 0 && strcmp(token[2], "temporal") != 0 && strcmp(token[2], "Linear-Nonlinear") != 0)
                    {
                        abort(line,"Expected any of the parameters for multimeter: 'spatial','temporal','Linear-Nonlinear'");
                        break;
                    }

                    // read parameters
                    if (token[5]){
                        if (strcmp(token[5], "{") == 0){
                            // check the module exists
                            continueReading=false;

                            if(strcmp("Input",token[4])==0){
                                continueReading=true;
                            }else{
                                for(int s=1;s<retina.getNumberModules();s++){
                                    if(retina.getModule(s)->checkID(token[4]))
                                        continueReading=true;
                                }
                            }


                            // select type
                            if(continueReading){
                                if (strcmp(token[2], "spatial") == 0 && token[6] && token[7] && token[8] && token[9] && token[10] && token[11] && token[12] && token[13] && token[14]){
                                    if(strcmp(token[6], "timeStep") == 0 && strcmp(token[8], "rowcol") == 0 && strcmp(token[10], "value") == 0 && strcmp(token[13], "Show") == 0){

                                        if(strcmp(token[9], "True") == 0)
                                            displayMg.addMultimeterTempSpat(token[3],token[4],atof(token[11]),atof(token[7]),false,token[14]);
                                        else if(strcmp(token[9], "False") == 0)
                                            displayMg.addMultimeterTempSpat(token[3],token[4],-atof(token[11]),atof(token[7]),false,token[14]);
                                        else
                                            continueReading=false;
                                    }else{
                                        abort(line,"Expected spatial multimeter parameter list: 'timeStep','rowcol','value','Show'");
                                        break;
                                    }
                                }else if(strcmp(token[2], "temporal") == 0 && token[6] && token[7] && token[8] && token[9] && token[10] && token[11] && token[12]){
                                    if(strcmp(token[6], "x") == 0 && strcmp(token[8], "y") == 0 && strcmp(token[11], "Show") == 0){
                                        displayMg.addMultimeterTempSpat(token[3],token[4],atof(token[7]),atof(token[9]),true,token[12]);
                                    }else{
                                        abort(line,"Expected temporal multimeter parameter list: 'x','y','Show'");
                                        break;
                                    }
                                }else if(strcmp(token[2], "Linear-Nonlinear") == 0 && token[6] && token[7] && token[8] && token[9] && token[10] && token[11] && token[12] && token[13] && token[14] && token[15] && token[16] && token[17] && token[18] && token[19] && token[20]){
                                    if(strcmp(token[6], "x") == 0 && strcmp(token[8], "y") == 0 && strcmp(token[10], "segment") == 0 && strcmp(token[12], "interval") == 0 && strcmp(token[14], "start") == 0 && strcmp(token[16], "stop") == 0 && strcmp(token[18], "Show") == 0){
                                        displayMg.addMultimeterLN(token[3],token[4],atof(token[7]),atof(token[9]),atof(token[11]),atof(token[13]),atof(token[15]),atof(token[17]),token[19]);
                                    }else{
                                        abort(line,"Expected Linear-Nonlinear multimeter parameter list: 'x','y','segment','interval','start','stop','Show'");
                                        break;
                                    }
                                }
                                else{
                                    abort(line,"Expected any of the multimeter types ('spatial', 'temporal', 'Linear-Nonlinear') and corresponding parameters");
                                    break;
                                }
                            }
                            else{
                                abort(line,"Parameter syntax error or specified module ID not found");
                                break;
                            }



                        }else{
                            abort(line,"Expected parameter list start token ('{')");
                            break;
                        }
                    }else{
                        abort(line,"Expected parameter token");
                        break;
                    }

                }else{
                    // Shouldn't we abort here?
                }

                action = 0;
                break;

            // Simulation time
            case 14:

                if (token[2]){
                    if (atof(token[2])>0)
                        retina.setSimTime(atof(token[2]));
                    else{
                        abort(line,"Expected a positive time value (>0)");
                        break;
                    }
                }else{
                    abort(line,"Expected simulation time value");
                    break;
                }

                if(verbose)cout << "Simulation time = "<< atof(token[2]) << endl;
                action = 0;

                break;

            // Number of trials
            case 15:

                if (token[2]){
                    if (atof(token[2])>0)
                        retina.setSimTotalRep(atof(token[2]));
                    else{
                        abort(line,"Expected a positive value (>0)");
                        break;
                    }
                }else{
                    abort(line,"Expected number of trials value");
                    break;
                }

                if(verbose)cout << "Number of trials = "<< atof(token[2]) << endl;
                action = 0;

                break;

            default:
                break;
            }
        }

    }//end while


    // close file
    fin.close();

}

//-------------------------------------------------//

void FileReader::abort(int line, char *error_msg){
    cout << "Incorrect syntax in line " << line << ": " << error_msg << endl;
    cout << "Aborting parsing of retina file." << endl;
    continueReading = false;

}


