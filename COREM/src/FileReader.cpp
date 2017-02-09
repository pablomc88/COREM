#include <string.h>
#include "FileReader.h"
#include "SequenceInput.h"
#include "StreamingInput.h"

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

bool FileReader::allocateValues(){
    bool ret_correct;
    fin.open(fileName);

    if (!fin.good()) {
        cout << "Could not open retina script file: " << fileName << ". Wrong retina file path?" << endl;
        CorrectFile = false;
        continueReading = false;
        ret_correct = false;
    } else
        ret_correct = true;
    return(ret_correct);
}

//-------------------------------------------------//

bool FileReader::nStringCopy(char *dest_buff, const char *src_str, size_t src_len, size_t dest_size){
    bool correct_copy;
    if(dest_size > 0 && dest_buff != NULL && src_str != NULL){ // Nothing to do if there is no space in destination buffer
        size_t chars_to_copy;
        
        if(src_len == (size_t)-1L) // If this paremeter is -1, we calculate the characters to copy from the source string length
            chars_to_copy = strlen(src_str);
        else
            chars_to_copy = src_len; // Copy the number of chars specified through parameter src_len
        
        if(chars_to_copy > dest_size-1){
            correct_copy=false; // Not enough space in destination buffer
            chars_to_copy = dest_size-1; // Copy as much characters as possible
        } else
            correct_copy=true;

        memcpy(dest_buff, src_str, chars_to_copy); // Copy specified chars excluding the \0
        dest_buff[chars_to_copy]='\0'; // Always add the \0 if there is some space in destinaton buffer
    } else
        correct_copy=false;
    return(correct_copy);
}

//-------------------------------------------------//

char *FileReader::getNextToken(const char *str, const char *especial_tokens){
    // fn state variables
    static char ret_token[MAX_CHARS_PER_LINE+1]; // Internal buffer which is used to store tokens and return the last one
    static const char *current_parsing_ptr = NULL; // Intenal fn state which remembers at which position of the input string the fn is parsing
    static char *current_token_ptr; // Pointer to the remaining space of ret_token
    // local variable 
    char *returned_token; // Pointer to the token returned 
    
    if(str == NULL){ // This fn has been called to inits its state (before first call to this fn for a new string)
        current_parsing_ptr = NULL; // Reset internal pointer (fn state)
        returned_token = NULL; // nothing to return
    } else {
        const char *next_token;

        if(current_parsing_ptr == NULL){ // If it is first time we call this fn for a new string and init, the internal state is not initiallized yet
           current_parsing_ptr = str; // Initialize internal state
           current_token_ptr = ret_token; // Empty buffer of tokens
        }

        returned_token = current_token_ptr; // The current (returned) token will be stored in space pointed by current_token_ptr

        next_token = strpbrk(current_parsing_ptr, especial_tokens); // Search for any of the tokens in str
        if(next_token != NULL) { // especial token found
            if(next_token > current_parsing_ptr) { // If there are characters before the found especial token:
                nStringCopy(returned_token, current_parsing_ptr, next_token-current_parsing_ptr, MAX_CHARS_PER_LINE-(returned_token-ret_token)); // copy them as a new token
                current_parsing_ptr = next_token; // Next parsing string position is the token found
            } else { // No char before found especial token
                nStringCopy(returned_token, next_token, 1, MAX_CHARS_PER_LINE-(returned_token-ret_token)); // copy especial token char as a new token
                current_parsing_ptr = next_token + 1; // Next parsing string position is following the especial token found
            }
        } else { // No more tokens
            size_t last_chars_len;
            last_chars_len = strlen(current_parsing_ptr);
            if(last_chars_len > 0) { // There was remaining chars in source string
                nStringCopy(returned_token, current_parsing_ptr, -1, MAX_CHARS_PER_LINE-(returned_token-ret_token)); // copy all them (-1) as a new token
                current_parsing_ptr += last_chars_len;
            } else // No remaining chars
                returned_token = NULL; // Return NULL
        }

        if(returned_token != NULL) // If a new token was found, update pointer to internal buffer
            current_token_ptr = returned_token + strlen(returned_token)+1; // Next token will be stored following the current one
    }
    return(returned_token);
}

//-------------------------------------------------//

bool FileReader::parseLine(const char *in_line, char **out_tokens, int max_tokens){

    const char * const espcial_tokens[] = { TOKENS_CMD, TOKENS_BLK, TOKENS_STR };
    enum context_t {CTX_CMD=0, CTX_BLK, CTX_STR};
    vector<enum context_t> context; // The status of the parser is a stack of contexts. Depending on the top context we use a different set of token delimiters

    int token_ind; // Index to the currently parsed token
    bool stop_parsing;

    context.push_back(CTX_CMD); // We start parsing in the action command context
    // parse subsequent tokens
    token_ind = 0;
    stop_parsing=false;
    getNextToken(NULL, NULL); // Reset line parser

    // Do while there is space in buffer and available tokens continue loop
    while(!stop_parsing && token_ind < max_tokens && (out_tokens[token_ind]=getNextToken(in_line, espcial_tokens[context.back()])) != NULL) { // get token
    
        // Process obtained token according to the current context:        
        switch(context.back()){
            
            case CTX_CMD: // This is the outer context
               switch(*out_tokens[token_ind]){ // Consider the first character of the token
                   case '#': // This a comment: stop parsing from here
                       out_tokens[token_ind] = NULL;
                       stop_parsing=true;
                       break;
                   case '{': // Start of block
                       context.push_back(CTX_BLK); // Change to block context
                       break;
                   case '}': // End of block
                       cout << "Syntax error: Found a } char which is not expected" << endl;
                       break;
                   case '\'': // Start of string
                       context.push_back(CTX_STR); // Change to string context
                       break;
                   case ' ': // Separators
                   case '\t':
                   case '.':
                       out_tokens[token_ind] = NULL;
                       token_ind--; // Discard these tokens in this context
                       break;
                   default: // Return the rest of tokens                       
                       break;
               }
               break;
            
            case CTX_BLK: // This is the block context
               switch(*out_tokens[token_ind]){
                   case '{': // Start of block
                       context.push_back(CTX_BLK); // Change to new block context
                       break;
                   case '}': // End of block
                       if(context.size() > 1) // There is more than one previous context
                           context.pop_back(); // Go back to previous context
                       else
                           cout << "Syntax error: Found a } char which is not expected" << endl;
                       break;
                   case '\'': // Start of string
                       context.push_back(CTX_STR); // Change to string context
                       break;
                   case ' ': // Separators
                   case '\t':
                       out_tokens[token_ind] = NULL;
                       token_ind--; // Discard these tokens in this context
                       break;
                   default: // Return the rest of tokens                       
                       break;
                }
                break;
            
            case CTX_STR: // This is the string context
               switch(*out_tokens[token_ind]){
                   case '\'': // End of string
                       if(context.size() > 1) // There is more than one previous context
                           context.pop_back(); // Go back to previous context
                       else
                           cout << "Syntax error: Found a ' char which is not expected" << endl;
                       break;
                   default: // Return the rest of tokens                       
                       break;
                }
                break;
        }
        token_ind++;
    }

    if(token_ind == max_tokens)
        if(max_tokens > 0)
            out_tokens[max_tokens-1] = NULL; // In case of implete parsing include token-list end mark
        
    return (token_ind < max_tokens); // Return false if there was not enough space in out_tokens
}

//-------------------------------------------------//

// This method remove all the specified tokens from a token list 
// token is a pointer to a list of pointers to string which contain the tokens to process. The
// last pointer of the pointer list must be NULL
// ign_tokens is a 0\-terminated string containing the tokens to discard
void FileReader::discardTokens(char **tokens, const char *ign_tokens){
    size_t n_token;
    size_t num_ign_tokens;
    
    num_ign_tokens = strlen(ign_tokens); // Number of tokens in the ignore list
    n_token=0;
    while(tokens[n_token]){ // For each of the input tokens
        size_t n_ign_token;
        if(strlen(tokens[n_token]) == 1){ // All tokens to ignore have length 1, check if current token has length 1
            // Search for the current input token in the token ignore list
            for(n_ign_token=0; n_ign_token < num_ign_tokens && tokens[n_token][0] != ign_tokens[n_ign_token]; n_ign_token++); // Compare all the ignored tokens to the current token and loop while not found
            if(n_ign_token < num_ign_tokens){ // Loop not finished, so current token has been found in the ignore list
                size_t rem_token;
                // Shift all the tokens in the list to remove the current token
                for(rem_token=n_token; tokens[rem_token]; rem_token++) // After this loop the input token list now has one token less
                    tokens[rem_token]=tokens[rem_token+1];
            } else
                n_token++; // Pass to he next input token
        } else
            n_token++;
    }
}

void FileReader::parseParameterBlock(char **block_tokens, module *new_module, int file_line) {
    vector<double> param_values;
    vector<string> param_ids;
    
    size_t next_tok_idx; // Next token to parse in the received 

    next_tok_idx = 0;
    // Check if we have more parameters to read
    if(continueReading && block_tokens[next_tok_idx]){
        if (strcmp(block_tokens[next_tok_idx], "{") == 0 && block_tokens[next_tok_idx+1]){ // parameter block start detected, read parameters
            
            next_tok_idx++; // skip block-start token
            while(continueReading && block_tokens[next_tok_idx]){
                if(strcmp(block_tokens[next_tok_idx], "}") == 0){ // parameter block end detected, end reading parameters
                    break; // exit while loop
                }else if (block_tokens[next_tok_idx+1]){ // There should be a value following the parameter name
                    param_ids.push_back(block_tokens[next_tok_idx]);
                    param_values.push_back(atof(block_tokens[next_tok_idx+1]));
                    next_tok_idx+=2;
                    
                    if (!block_tokens[next_tok_idx]) // After each parameter-value tuple we should always find a token (at least block end)
                        abort(file_line,"Incorrect parameter block format: block-end character not found (at least in a correct place)");
                }else
                    abort(file_line,"Incorrect parameter block format: apparent parameter name without value"); // abort() sets continueReading to false
            }
        } else
            abort(file_line,"Incorrect start of parameter list");
    }
    // Set parsed module parameters
    if(continueReading) {
        continueReading = new_module->setParameters(param_values, param_ids);
        if(!continueReading)
            abort(file_line,"Error setting specified module parameters (incorrect parameter name or invalid value)");
    }
}


//-------------------------------------------------//

void FileReader::parseFile(Retina &retina, DisplayManager &displayMg){
    bool verbose = false;
    int line = 0;
    int action = 0;

    while (!fin.eof() && CorrectFile && continueReading){
        // read a line into memory
        char line_buf[MAX_CHARS_PER_LINE];
        // array to store memory addresses of the tokens in line_buf
        char *token[MAX_TOKENS_PER_LINE];
        
        line++;
        fin.getline(line_buf, MAX_CHARS_PER_LINE); // Endline char not included

        if(parseLine(line_buf, token, MAX_TOKENS_PER_LINE))
            discardTokens(token, "'(),"); // These tokens are not meaningfull for the script syntax
        else
            abort(line,"Check tokens and delimiter characters");
        
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
                        else if( strcmp(token[1], "Output") == 0 ){
                            action = 16;
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
            
            if(action==11 || action==13) // If Action is Show or multimeter prepare Display Manager
                displayMg.allocateValues(retina.getNumberModules(),retina.getStep()); // allocate values of Display Manager even if no Connect command is used
                
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
                                    // Several values of E
                                    }else if( strcmp(token[i], "E") == 0 && strcmp(token[i+1], "{") == 0){
                                        while(strcmp(token[i+2], "}") != 0 ){
                                            pid.push_back("E");
                                            p.push_back(atof(token[i+2]));
                                            i+=1;
                                        }
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
                                    abort(line,"Incorrect parameter format");
                                    break;
                                }
                            }
                            // Add module to the retina
                            if(continueReading){
                                continueReading=newModule->setParameters(p,pid);

                                if(continueReading){
                                    retina.addModule(newModule,token[3]);
                                }else{
                                    abort(line,"Error setting specified parameters (incorrect name or value)");
                                    break;
                                }
                            }

                        }else{
                            abort(line,"Incorrect start of parameter list");
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
                                    abort(line,"Neither '+' nor '-' operator found");
                                    continueReading=false;
                                    break;
                                }
                                change=true;
                            }
                            i+=1;
                            if (!token[i]){
                                abort(line,"Expected token not found in Connect parameter block");
                                continueReading=false; // Syntax error: do not continue
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
                            abort(line,"Expected target module ID and connection type ('Current'/'Conductante') after list of source module IDs");
                            break;
                        }

                    }
                    // Only one input
                    else{
                        modulesID.push_back(token[2]);
                        continueReading=retina.connect(modulesID,token[3],operations,token[4]);
                        if (!continueReading){
                            abort(line,"Incorrect module IDs for Connect action command");
                            break;
                        }
                        if(verbose)cout << "Module " << token[2] << " connected to " << token[3] << endl;
                    }
                }else{
                    abort(line,"Not enough parameters for Connect");
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
                if (token[2] && token[3]){
                    // Input sequence
                    if(strcmp(token[2], "grating") == 0 ){

                        if (strcmp(token[3], "{") == 0 && strcmp(token[4],"type")==0 && strcmp(token[6],"step")==0 && strcmp(token[8],"length1")==0 && strcmp(token[10],"length2")==0 && strcmp(token[12],"length3")==0 && strcmp(token[14],"sizeX")==0 && strcmp(token[16],"sizeY")==0 && strcmp(token[18],"freq")==0 && strcmp(token[20],"period")==0 && strcmp(token[22],"Lum")==0 && strcmp(token[24],"Contr")==0 && strcmp(token[26],"phi_s")==0 && strcmp(token[28],"phi_t")==0 && strcmp(token[30],"orientation")==0 && strcmp(token[32],"red_weight")==0 && strcmp(token[34],"green_weight")==0 && strcmp(token[36],"blue_weight")==0 && strcmp(token[38],"red_phase")==0 && strcmp(token[40],"green_phase")==0 && strcmp(token[42],"blue_phase")==0 && strcmp(token[44],"}")==0){
                            continueReading=retina.generateGrating(atof(token[5]),atof(token[7]),atof(token[9]),atof(token[11]),atof(token[13]),atof(token[15]),atof(token[17]),atof(token[19]),atof(token[21]),atof(token[23]),atof(token[25]),atof(token[27]),atof(token[29]),atof(token[31]),atof(token[33]),atof(token[35]),atof(token[37]),atof(token[39]),atof(token[41]),atof(token[43]));
                            if(verbose)cout << "Grating generated." << endl;
                        }else{
                            abort(line,"Expected parameter list of grating: 'type','step','length1','length2','length3','sizeX','sizeY','freq','period','Lum','Contr','phi_s','phi_t','orientation','red_weight','green_weight','blue_weight','red_phase','green_phase','blue_phase'");
                            break;
                        }
                    }else if(strcmp(token[2], "fixationalMovGrating") == 0 ){

                        if (strcmp(token[3], "{") == 0 && strcmp(token[4],"sizeX")==0 && strcmp(token[6],"sizeY")==0 && strcmp(token[8],"circle_radius")==0 && strcmp(token[10],"jitter_period")==0 && strcmp(token[12],"spatial_period")==0 && strcmp(token[14],"step_size")==0 && strcmp(token[16],"Lum")==0 && strcmp(token[18],"Contr")==0 && strcmp(token[20],"orientation")==0 && strcmp(token[22],"red_weight")==0 && strcmp(token[24],"green_weight")==0 && strcmp(token[26],"blue_weight")==0 && strcmp(token[28],"type1")==0 && strcmp(token[30],"type2")==0 && strcmp(token[32],"switch")==0 && strcmp(token[34],"}")==0){
                            continueReading=retina.generateFixationalMovGrating(atof(token[5]),atof(token[7]),atof(token[9]),atof(token[11]),atof(token[13]),atof(token[15]),atof(token[17]),atof(token[19]),atof(token[21]),atof(token[23]),atof(token[25]),atof(token[27]),atof(token[29]),atof(token[31]),atof(token[33]));
                            if(verbose)cout << "Grating of fixational movements generated." << endl;
                        }else{
                            abort(line,"Expected parameter list of fixationalMovGrating: 'sizeX','sizeY','circle_radius','jitter_period','spatial_period','step_size','Lum','Contr','orientation','red_weight','green_weight','blue_weight','type1','type2','switch'");
                            break;
                        }
                    }else if(strcmp(token[2], "whiteNoise") == 0 ){
                        if (strcmp(token[3], "{") == 0 && strcmp(token[4],"mean")==0 && strcmp(token[6],"contrast1")==0 && strcmp(token[8],"contrast2")==0 && strcmp(token[10],"period")==0 && strcmp(token[12],"switch")==0 && strcmp(token[14],"sizeX")==0 && strcmp(token[16],"sizeY")==0 && strcmp(token[18],"}")==0){
                            continueReading=retina.generateWhiteNoise(atof(token[5]),atof(token[7]),atof(token[9]),atof(token[11]),atof(token[13]),atof(token[15]),atof(token[17]));
                            if(verbose)cout << "White noise generated." << endl;
                        }else{
                            abort(line,"Expected parameter list of whiteNoise: 'mean','contrast1','contrast2','period','switch','sizeX','sizeY'");
                            break;
                        }
                    }else if(strcmp(token[2], "impulse") == 0 ){
                        if (strcmp(token[3], "{") == 0 && strcmp(token[4],"start")==0 && strcmp(token[6],"stop")==0 && strcmp(token[8],"amplitude")==0 && strcmp(token[10],"offset")==0 && strcmp(token[12],"sizeX")==0 && strcmp(token[14],"sizeY")==0 && strcmp(token[16],"}")==0){
                            continueReading=retina.generateImpulse(atof(token[5]),atof(token[7]),atof(token[9]),atof(token[11]),atof(token[13]),atof(token[15]));
                            if(verbose)cout << "Impulse generated." << endl;
                        }else{
                            abort(line,"Expected parameter list of impulse: 'start','stop','amplitude','offset','sizeX','sizeY'");
                            break;
                        }
                    }else if(strcmp(token[2], "sequence") == 0 ){
                        module *new_input_module;
                        new_input_module = new SequenceInput(retina.getSizeX(), retina.getSizeY(), retina.getStep(), token[3]);
                        
                        parseParameterBlock(token+4, new_input_module, line);
                        
                        // Add module to the retina
                        if(continueReading) {
                            retina.addModule(new_input_module, "Input");
                            retina.setModuleInput();
                            if(verbose) cout << "Input configured as sequence of frames." << endl;
                        } else 
                            break;
                    }else if(strcmp(token[2], "streaming") == 0 ){
                        module *new_input_module;
                        new_input_module = new StreamingInput(retina.getSizeX(), retina.getSizeY(), retina.getStep(), token[3]);
                        
                        parseParameterBlock(token+4, new_input_module, line);
                        
                        // Add module to the retina
                        if(continueReading) {
                            retina.addModule(new_input_module, "Input");
                            retina.setModuleInput();
                            if(verbose) cout << "Input configured as streaming video." << endl;
                        } else 
                            break;
                    }else{
                        abort(line,"Unknown input type");
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
                    for(int l=1;l<retina.getNumberModules();l++){ // For all retina modules except 0 (Input) with specified ID set isShown
                        module *m = retina.getModule(l);
                        string ID = m->getModuleID();
                        if(ID.compare(token[2])==0){
                            continueReading=true;
                            module_id_found=true;
                            if(strcmp(token[3],"True")==0){
                                displayMg.setIsShown(true,l);
                                if(verbose)cout << "Module "<<  token[2] <<" is displayed." << endl;
                                // also input
                                //displayMg.setIsShown(true,0);
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
                        retina.setTotalSimTime(atof(token[2]));
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
                        retina.setSimTotalTrials(atof(token[2]));
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

            // Output
            case 16:
                if(token[2] && token[3]){ // Check that at least we can read the output type and its ID
                    if(strstr(token[3], "Output") == token[3]){ // Check if the specified module ID contains the substring Output at the beggining of the ID
                        // read module type
                        int next_tok_idx; // Next token to parse
                        module *newModule;
                        vector<double> p;
                        vector<string> pid; // Parameter list
                        
                        if (strcmp(token[2], "spikes") == 0 ) {
                            string output_filename;
                            
                            if (token[4] && strcmp(token[4], "{") != 0){ // the next token is not {, assume that it is the output filename
                                output_filename=token[4]; // Replace (default) filename
                                next_tok_idx=5; // Pass to the next token to continue reading parameters
                            } else {
                                output_filename=""; // Use the default filename
                                next_tok_idx=4; // continue reading parameters from this current token
                            }
                            newModule = new SpikingOutput(retina.getSizeX(), retina.getSizeY(), retina.getStep(), output_filename);
                        }
                        else if (strcmp(token[2], "sequence") == 0 ) {
                            string output_filename;
                            
                            if (token[4] && strcmp(token[4], "{") != 0){ // the next token is not {, assume that it is the output filename
                                output_filename=token[4]; // Replace (default) filename
                                next_tok_idx=5; // Pass to the next token to continue reading parameters
                            } else {
                                output_filename=""; // Use the default filename
                                next_tok_idx=4; // continue reading parameters from this current token
                            }
                            newModule = new SequenceOutput(retina.getSizeX(), retina.getSizeY(), retina.getStep(), output_filename);
                        } else {
                            abort(line,"Unknown retina output type");
                            break;
                        }
                      
                        // Process parameter block if it is found
                        parseParameterBlock(token+next_tok_idx, newModule, line);

                        // Add module to the retina
                        if(continueReading) {
                            retina.addModule(newModule,token[3]);
                            if(verbose) cout << "Output module " << token[3] << "added to the retina" << endl;
                        } else
                            break;
                    } else {
                        abort(line,"The ID of Output module must start with the characters 'Output'");
                        break;
                    }                    
                }else{
                    abort(line,"Expected a retina output type and output module ID");
                    break;
                }

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

void FileReader::abort(int line, const char *error_msg){
    cout << "Incorrect syntax in line " << line << ": " << error_msg << endl;
    cout << "Aborting parsing of retina file." << endl;
    continueReading = false;

}


