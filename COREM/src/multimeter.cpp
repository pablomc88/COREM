#include "multimeter.h"

// Constants for the display
const unsigned char color1[] = {255,0,0};
const unsigned char color2[] = {0,0,255};
const unsigned char backColor[] = {255,255,255};

multimeter::multimeter(int x,int y,double step){

    sizeX = x;
    sizeY = y;
    simStep = step;
    drawDisp = new CImgDisplay();
    recordAllCells = False;
}

multimeter::multimeter(const multimeter& copy){

    sizeX = copy.sizeX;
    sizeY = copy.sizeY;
    simStep = copy.simStep;
    drawDisp = new CImgDisplay(*copy.drawDisp);
    recordAllCells = copy.recordAllCells;
}

multimeter::~multimeter(){
    delete drawDisp;
    timeRecord.clear();
}

void multimeter::setSizeX(int x){
    sizeX = x;
}

void multimeter::setSizeY(int y){
    sizeY = y;
}

void multimeter::setStep(double step){
    simStep = step;
}

void multimeter::setRecordAllCells(bool value){
    recordAllCells = value;
}

void multimeter::setStartTime(double value){
    startTime = value;
}

void multimeter::setRangeToPlot(double value){
    rangeToPlot = value;
}

bool multimeter::getRecordAllCells(){
    return recordAllCells;
}

void multimeter::initializeTimeRecord(){

    for (int i = 0; i < sizeX*sizeY; i++) {
        vector<double> row; // Create an empty row
        row.push_back(0.0); // Add an element (column) to the row
        timeRecord.push_back(row); // Add the row to the main vector
    }

}

void multimeter::initializeLNAnalysis(int numberTrials){

    for (int i = 0; i < numberTrials; i++) {
        vector<double> row1; // Create an empty row
        row1.push_back(0.0); // Add an element (column) to the row
        LN_timeRecord.push_back(row1); // Add the row to the main vector

        vector<double> row2; // Create an empty row
        row2.push_back(0.0); // Add an element (column) to the row
        LN_input.push_back(row2); // Add the row to the main vector
    }
}


void multimeter::recordValue(double value,int cell){
    if (recordAllCells)
        timeRecord[cell].push_back(value);
    else
        timeRecord[0].push_back(value);

}

void multimeter::recordInput(double value){
    input.push_back(value);
}

void multimeter:: recordValueLNAnalysis(double value,int trial){
    LN_timeRecord[trial].push_back(value);
}

void multimeter:: recordInputLNAnalysis(double value,int trial){
    LN_input[trial].push_back(value);
}

string multimeter::getWorkingDir(){

    char* cwd;
    char buff[PATH_MAX + 1];

    cwd = getcwd( buff, PATH_MAX + 1 );
    string currentDir(cwd);
    string currentDirRoot = currentDir+"/";

    return currentDirRoot;
}

void multimeter::saveArray(double array[],int size,string fileID){

    ofstream myfile (getWorkingDir() + "results/"+fileID);

    if (myfile.is_open())
    {
        for(int count = 0; count < size; count ++){
            myfile << array[count] << endl;
        }
        myfile.close();
    }
      else cout << "Unable to save the file "+fileID;

}

vector<double> multimeter::loadArray(string fileID){

    // Load file content into a vector
    ifstream myfile (getWorkingDir() + "results/"+fileID);
    vector<double> loadVector;

    if (myfile.is_open()) {
        while (!myfile.eof())
        {
            char buf[1000];
            myfile.getline(buf,1000);
            const char* token[1] = {};
            token[0] = strtok(buf, "\n");
            if(token[0] != NULL)
                loadVector.push_back(atof(token[0]));
        }
        myfile.close();
    }
    else cout << "Unable to open the file "+fileID;

    return loadVector;
}

void multimeter::saveAllVectors(int trial){

    double LN_input_array [LN_input[trial].size()];
    double LN_timeRecord_array [LN_timeRecord[trial].size()];

    for (int i=0;i<LN_input[trial].size();i++)
        LN_input_array[i] = LN_input[trial][i];

    for (int i=0;i<LN_timeRecord[trial].size();i++)
        LN_timeRecord_array[i] = LN_timeRecord[trial][i];

    saveArray(LN_input_array,LN_input[trial].size(),"LN_input_"+to_string(trial));
    saveArray(LN_timeRecord_array,LN_timeRecord[trial].size(),"LN_timeRecord_"+to_string(trial));

}


void multimeter::loadAllVectors(int numberTrials){

    for (int i = 0; i < numberTrials; i++) {

        vector<double> LN_input_vector = loadArray("LN_input_"+to_string(i));
        vector<double> LN_timeRecord_vector = loadArray("LN_timeRecord_"+to_string(i));

        LN_input[i].clear();
        LN_timeRecord[i].clear();

        for (int k=0;k<LN_input_vector.size();k++)
            LN_input[i].push_back(LN_input_vector[k]);

        for (int k=0;k<LN_timeRecord_vector.size();k++)
            LN_timeRecord[i].push_back(LN_timeRecord_vector[k]);

    }

}


void multimeter::showSpatialProfile(CImg<double> *img, bool rowCol, int cell,
                                    string title, int col, int row, bool lastWindow,
                                    bool showDisplay, string fileID){

    double imSize = 0.0;

    // Create a one-row image with the values of one row/col from the input image
    if (rowCol == true)
        imSize = img->width();
    else
        imSize = img->height();

    CImg <double> *spatialPlot = new CImg <double>(imSize,1,1,1,0);
    double arrayToFile[int(imSize)];
    double max_value = -DBL_INF;
    double min_value = DBL_INF;

    for (int k=0;k<imSize;k++){

        if (rowCol == true)
            (*spatialPlot)(k,0,0,0) = (*img)(k,cell,0,0);
        else
            (*spatialPlot)(k,0,0,0) = (*img)(cell,k,0,0);

        arrayToFile[k] = (*spatialPlot)(k,0,0,0);

        // Maximum and minimum used for normalization
        if ((*spatialPlot)(k,0,0,0) > max_value)
            max_value = (*spatialPlot)(k,0,0,0);
        if ((*spatialPlot)(k,0,0,0) < min_value)
            min_value = (*spatialPlot)(k,0,0,0);
    }

    // to avoid errors with the display
    if(max_value == min_value){
        max_value+=1;
        min_value-=1;
    }

    // Save results to file
    saveArray(arrayToFile,int(imSize),fileID);

    // Plot
    if(showDisplay){
        CImg <unsigned char> *display = new CImg <unsigned char>(400,256,1,3,0);
        display->fill(*backColor);
        const char * titleChar = (title).c_str();
        drawDisp->assign(*display,titleChar);

        display->draw_graph((spatialPlot->get_crop(0,0,0,0,imSize-1,0,0,0)-min_value)*255/(max_value-min_value)
                            ,color1,1,1,4,255,0).display(*drawDisp);
//        display->draw_axes(0.0,imSize,max_value,min_value,color2,1,-80,-80,0,0,~0U,~0U,20).display(*drawDisp);
//        display->draw_text(320,200,"cell",color2,backColor,1,20).display(*drawDisp);
//        display->draw_text(40,5,"Output",color2,backColor,1,20).display(*drawDisp);

        // axes
        CImg <double> *x_axis = new CImg <double>(3,1,1,1,0);
        (*x_axis)(0,0,0,0) = -imSize/2.0;
        (*x_axis)(1,0,0,0) = 0;
        (*x_axis)(2,0,0,0) = imSize/2.0;;

        CImg <double> *y_axis = new CImg <double>(1,3,1,1,0);
        (*y_axis)(0,0,0,0) = max_value;
        (*y_axis)(0,1,0,0) = (max_value-min_value)/2 + min_value;
        (*y_axis)(0,2,0,0) = min_value;

        display->draw_axis(*x_axis,226,color2,1,~0U,20,true).display(*drawDisp);
        display->draw_axis(110,*y_axis,color2,1,~0U,20,true).display(*drawDisp);
        display->draw_text(340,190,"cell",color2,backColor,1,20).display(*drawDisp);
        display->draw_text(120,10,"Output",color2,backColor,1,20).display(*drawDisp);

        // move display
        drawDisp->move(col,row);

        if(lastWindow){
            cout << "\rClose spatial multimeter window to continue..." << endl;
            while (!drawDisp->is_closed())
                drawDisp->wait();
        }
        delete display;
    }
    delete spatialPlot;

}

void multimeter::showTimeProfile(string title, int col, int row, bool lastWindow,
                                    bool showDisplay, string fileID){

    // Size of the time array in simulation steps.
    // The first position of the array is not taken (-1)
    int size = int(timeRecord[0].size()-1-startTime/simStep);

    CImg <double> *timePlot = new CImg <double>(size,1,1,1,0);
    double arrayToFile[size];
    double max_value = -DBL_INF;
    double min_value = DBL_INF;

    for (int k=0;k<size;k++){

        (*timePlot)(k,0,0,0) = (timeRecord[0])[k+1+int(startTime/simStep)];
        arrayToFile[k] = (*timePlot)(k,0,0,0);

        // Maximum and minimum used for normalization
        if ((*timePlot)(k,0,0,0) > max_value)
            max_value = (*timePlot)(k,0,0,0);
        if ((*timePlot)(k,0,0,0) < min_value)
            min_value = (*timePlot)(k,0,0,0);
    }

    // to avoid errors with the display
    if(max_value == min_value){
        max_value+=1;
        min_value-=1;
    }

    // Save results to file
    if (recordAllCells){
        cout << "saving responses of all cells..." << endl;

        for(int cell=0;cell<sizeX*sizeY;cell++){
            int size = int(timeRecord[cell].size()-1-startTime/simStep);
            double temp[size];
            for(size_t k=0;k<size;k++){
                temp[k] = timeRecord[cell][k+1+int(startTime/simStep)];
            }

            string cc = to_string(cell);
            saveArray(temp,size,fileID + cc);
        }

    }else
        saveArray(arrayToFile,size,fileID);

    // Plot
    if(showDisplay){
        CImg <unsigned char> *display = new CImg <unsigned char>(400,256,1,3,0);
        display->fill(*backColor);
        const char * titleChar = (title).c_str();
        drawDisp->assign(*display,titleChar);

        display->draw_graph((timePlot->get_crop(0,0,0,0,size-1,0,0,0)-min_value)*255/(max_value-min_value)
                            ,color1,1,1,4,255,0).display(*drawDisp);

        // axes
        CImg <double> *x_axis = new CImg <double>(3,1,1,1,0);
        (*x_axis)(0,0,0,0) = startTime;
        (*x_axis)(1,0,0,0) = startTime + ((timeRecord[0].size()-1)*simStep -
                startTime)/2;
        (*x_axis)(2,0,0,0) = (timeRecord[0].size()-1)*simStep;

        CImg <double> *y_axis = new CImg <double>(1,3,1,1,0);
        (*y_axis)(0,0,0,0) = max_value;
        (*y_axis)(0,1,0,0) = (max_value-min_value)/2 + min_value;
        (*y_axis)(0,2,0,0) = min_value;

        display->draw_axis(*x_axis,226,color2,1,~0U,20,true).display(*drawDisp);
        display->draw_axis(110,*y_axis,color2,1,~0U,20,true).display(*drawDisp);
        display->draw_text(300,190,"Time (ms)",color2,backColor,1,20).display(*drawDisp);
        display->draw_text(120,10,"Output",color2,backColor,1,20).display(*drawDisp);

        // move display
        drawDisp->move(col,row);

        if(lastWindow){
            cout << "\rClose spatial multimeter window to continue..." << endl;
            while (!drawDisp->is_closed())
                drawDisp->wait();
        }
        delete display;
    }
    delete timePlot;

}


void multimeter::showLNAnalysis(int col, int row, bool lastWindow, bool showDisplay, string fileID, double segment, double interval, double start, double stop, int numberTrials){

    cout << "LN analysis" << endl;

    // The filter F is computed as the correlation between s(t) and the response
    // r(t), normalized by the autocorrelation of the stimulus in the Fourier
    // domain

    // calculate NFFT as the next higher power of 2 >= Nx.
    int NFFT = (int)pow(2.0, ceil(log(double(segment))/log(2.0)));

    // allocate memory for NFFT complex numbers
    double *R = (double *) malloc((2*NFFT+1) * sizeof(double));
    double *S = (double *) malloc((2*NFFT+1) * sizeof(double));
    double *S_conj = (double *) malloc((2*NFFT+1) * sizeof(double));
    double *numerator = (double *) malloc((2*NFFT+1) * sizeof(double));
    double *denominator = (double *) malloc((2*NFFT+1) * sizeof(double));
    double *F = (double *) malloc((2*NFFT+1) * sizeof(double));

    R[0] = 0.0;
    S[0] = 0.0;
    S_conj[0] = 0.0;
    numerator[0] = 0.0;
    denominator[0] = 0.0;
    F[0] = 0.0;

    // -> 'start' and 'stop' are the start and end simulation times of the
    // recording used for computing the LN analysis.
    // -> 'segment' is the length of the time window where the filter F is calculated
    // (typically 1000 ms).
    // -> F is averaged over all trials and segments spaced every 'interval' ms
    // throughout the recording.

    int number_rep = int((stop-start)/interval); // number of segments between
                                                // start and stop
    double sum_s = 0;
    double sum_r = 0;
    double mean_s = 0;
    double mean_r = 0;
    vector <double> s; // input stimulus
    vector <double> r; // cell response

    // Average F for all trials
    for(int trial = 0;trial < numberTrials;trial++){

        // Reset vectors
        s.clear();
        r.clear();

        // The stimulus intensity is normalized to have zero mean and a standard
        // deviation equal to the contrast. The response is normalized to have
        // zero mean
        sum_s = accumulate(LN_input[trial].begin(), LN_input[trial].end(), 0.0);
        mean_s = sum_s / LN_input[trial].size();
        sum_r = accumulate(LN_timeRecord[trial].begin(), LN_timeRecord[trial].end(), 0.0);
        mean_r = sum_r / LN_timeRecord[trial].size();

        for (size_t k=0;k<LN_input[trial].size()-1;k++)
            s.push_back(LN_input[trial][k+1]-mean_s);

        for (size_t k=0;k<LN_timeRecord[trial].size()-1;k++){
            r.push_back(LN_timeRecord[trial][k+1]-mean_r);
        }

        // Normalize input to have a standard deviation equal to the contrast
        // (input values are between 0 and 255)
        for (size_t k=0;k<s.size();k++)
            s[k] /= double(255);

        // Average F for all segments
        for (int rep=0;rep<number_rep;rep++){
            // Storing vectors in a complex array. This is needed even though x(n)
            // is purely real in this case.
            for(int i=0; i<segment; i++)
            {
                R[2*i+1] = r[start + rep*interval + i];
                R[2*i+2] = 0.0;

                S[2*i+1] = s[start + rep*interval + i];
                S[2*i+2] = 0.0;

                S_conj[2*i+1] = 0.0;
                S_conj[2*i+2] = 0.0;

            }
            // pad the remainder of the array with zeros (0 + 0 j)
            for(int i=segment; i<NFFT; i++)
            {
                R[2*i+1] = 0.0;
                R[2*i+2] = 0.0;

                S[2*i+1] = 0.0;
                S[2*i+2] = 0.0;

                S_conj[2*i+1] = 0.0;
                S_conj[2*i+2] = 0.0;
            }

            // FFT
            fft(R, NFFT, 1);
            fft(S, NFFT, 1);
            conj(S,S_conj,NFFT);

            for(int i=0; i<NFFT; i++)
            {
                // Initialize for the first trial and first segment
                if(rep==0 && trial==0){
                    numerator[2*i+1] = (S_conj[2*i+1]*R[2*i+1]) - (S_conj[2*i+2]*R[2*i+2]);
                    numerator[2*i+2] = (S_conj[2*i+1]*R[2*i+2]) + (S_conj[2*i+2]*R[2*i+1]);

                    denominator[2*i+1] = (S_conj[2*i+1]*S[2*i+1]) - (S_conj[2*i+2]*S[2*i+2]);
                    denominator[2*i+2] = (S_conj[2*i+1]*S[2*i+2]) + (S_conj[2*i+2]*S[2*i+1]);

                }else{
                    numerator[2*i+1] += (S_conj[2*i+1]*R[2*i+1]) - (S_conj[2*i+2]*R[2*i+2]);
                    numerator[2*i+2] += (S_conj[2*i+1]*R[2*i+2]) + (S_conj[2*i+2]*R[2*i+1]);

                    denominator[2*i+1] += (S_conj[2*i+1]*S[2*i+1]) - (S_conj[2*i+2]*S[2*i+2]);
                    denominator[2*i+2] += (S_conj[2*i+1]*S[2*i+2]) + (S_conj[2*i+2]*S[2*i+1]);
                }

            }

        }
    }


    for(int i=0; i<NFFT; i++)
    {
        numerator[2*i+1] /= (number_rep * numberTrials);
        numerator[2*i+2] /= (number_rep * numberTrials);
        denominator[2*i+1] /= (number_rep * numberTrials);
        denominator[2*i+2] /= (number_rep * numberTrials);

        F[2*i+1] = ((numerator[2*i+1]*denominator[2*i+1]) + (numerator[2*i+2]*denominator[2*i+2]))/((denominator[2*i+1])*(denominator[2*i+1]) + (denominator[2*i+2])*(denominator[2*i+2]) + DBL_EPSILON);
        F[2*i+2] = ((numerator[2*i+2]*denominator[2*i+1]) - (numerator[2*i+1]*denominator[2*i+2]))/((denominator[2*i+1])*(denominator[2*i+1]) + (denominator[2*i+2])*(denominator[2*i+2]) + DBL_EPSILON);
    }

    // iFFT of F
    fft(F, NFFT, -1);

    // normalize the IFFT of F
    for(int i=0; i<NFFT; i++)
    {
        F[2*i+1] /= NFFT;
        F[2*i+2] /= NFFT;
    }

    // All samples between start and stop for all trials are now convolved with the
    // filter F to get g(t): g(t) = F*s(t). This convolution is again computed
    // using the FFT
    int newNFFT = (int)pow(2.0, ceil(log(double(numberTrials*(stop-start)))/log(2.0)));

    double *G = (double *) malloc((2*newNFFT+1) * sizeof(double));
    double *newS = (double *) malloc((2*newNFFT+1) * sizeof(double));
    double *newF = (double *) malloc((2*newNFFT+1) * sizeof(double));

    G[0]=0.0;
    newS[0]=0.0;
    newF[0]=0.0;

    double varianceS = 0; // to compute the variance of the stimulus

    int trial = 0;
    int m = 0; // index to traverse samples between start and stop

    for(int i=0; i < numberTrials*(stop-start); i++)
    {
        // Reset stimulus for each trial
        if (m==0){
            s.clear();
            sum_s = accumulate(LN_input[trial].begin(), LN_input[trial].end(), 0.0);
            mean_s = sum_s / LN_input[trial].size();

            for (size_t k=0;k<LN_input[trial].size()-1;k++)
                s.push_back(LN_input[trial][k+1]-mean_s);

            for (size_t k=0;k<s.size();k++)
                s[k] /= double(255);
        }

        newS[2*i+1] = s[start + m];
        newS[2*i+2] = 0.0;

        G[2*i+1] = 0.0;
        G[2*i+2] = 0.0;

        varianceS += s[start + m]*s[start + m];

        if (m<(stop-start) - 1)
            m+=1;
        else{
            m=0;
            trial+=1;
        }

    }

    for(int i= numberTrials*(stop-start); i<newNFFT; i++)
    {
        newS[2*i+1] = 0.0;
        newS[2*i+2] = 0.0;

        G[2*i+1] = 0.0;
        G[2*i+2] = 0.0;
    }

    for(int i=0; i<newNFFT; i++)
    {
        if(i<NFFT){
            newF[2*i+1]=F[2*i+1];
            newF[2*i+2]=F[2*i+2];
        }else{
            newF[2*i+1]=0.0;
            newF[2*i+2]=0.0;
        }
    }

    // FFT
    fft(newF, newNFFT, 1);
    fft(newS, newNFFT, 1);

    // Convolution
    for(int i=0; i<newNFFT; i++)
    {
        G[2*i+1] = (newF[2*i+1]*newS[2*i+1]) - (newF[2*i+2]*newS[2*i+2]);
        G[2*i+2] = (newF[2*i+1]*newS[2*i+2]) + (newF[2*i+2]*newS[2*i+1]);
    }

    // iFFT of g
    fft(G, newNFFT, -1);

    // normalize the IFFT of g
    for(int i=0; i<newNFFT; i++)
    {
        G[2*i+1] /= newNFFT;
        G[2*i+2] /= newNFFT;

    }

    // Scale the filter in amplitude so that the variance of the filtered
    // stimulus, g(t), was equal to the variance of the stimulus, s(t).
    // Note that mean values of g(t) and s(t) are 0.

    double varianceG = 0;

    for (int k=0;k<newNFFT;k++) {
        varianceG += G[2*k+1]*G[2*k+1];
    }

    varianceS /= (numberTrials*(stop-start));
    varianceG /= newNFFT;

    for(int k=0;k<newNFFT;k++)
        G[2*k+1]*=sqrt(varianceS/varianceG);

    for(int k=0;k<NFFT;k++)
        F[2*k+1]*=sqrt(varianceS/varianceG);

    // Min and max values of G
    double max_g = -DBL_INF;
    double min_g = DBL_INF;

    for(int k=0;k<newNFFT;k++){
        if(G[2*k+1] < min_g)
            min_g = G[2*k+1];
        if(G[2*k+1] > max_g)
            max_g = G[2*k+1];
    }

    if (min_g == max_g){
        min_g-=1;
        max_g+=1;
    }

    // Then, the fixed nonlinearity N(g) is calculated by plotting r(t)
    // against g(t) and averaging the values of r over bins of g

    // Histogram of g
    double number_bins = 1000;
    double histogram[int(number_bins)];
    int histogram_count[int(number_bins)];
    double pos_hist = 0;

    for (size_t k=0;k<number_bins;k++){
        histogram[k] = 0;
        histogram_count[k] = 0;
    }

    m = 0;
    trial = 0;

    for(int i=0; i < numberTrials*(stop-start); i++)
    {
        // Reset r
        if (m==0){
            r.clear();
            for (size_t k=0;k<LN_timeRecord[trial].size()-1;k++)
                r.push_back(LN_timeRecord[trial][k+1]);
        }

        // Update histogram
        pos_hist = (G[2*i+1] - min_g)/(max_g - min_g);
        histogram[int(pos_hist*(number_bins-1))] += r[start + m];
        histogram_count[int(pos_hist*(number_bins-1))] += 1;

        if (m<(stop-start) - 1)
            m+=1;
        else{
            m=0;
            trial+=1;
        }

    }

    // Normalize
    for (size_t k=0;k<number_bins;k++){
        if (histogram_count[k]>0)
            histogram[k] /= double(histogram_count[k]);
    }

    // Interpolation and discard of extreme values
    double window = number_bins/10; // Number of bins used for the interpolation
    double discard_bins = number_bins/4; // Number of bins to discard on each side
    double histogram_interpolated[int(number_bins-window-2*discard_bins+1)];

    for (size_t k=window/2+discard_bins;k<=number_bins-window/2-discard_bins;k++){
        for(int j=-window/2;j<=window/2;j++)
            histogram_interpolated[int(k-window/2-discard_bins)]+=histogram[k+j];
    }

    for (size_t k=0;k<number_bins-window-2*discard_bins+2;k++)
        histogram_interpolated[k] /= double(window + 1);

    // Plot
    int size_F = int(rangeToPlot/simStep);
    int size_NL = int(number_bins-window-2*discard_bins+1);

    CImg <double> *LNPlot_F = new CImg <double>(size_F,1,1,1,0);
    CImg <double> *LNPlot_NL = new CImg <double>(size_NL,1,1,1,0);

    double max_value_F = -DBL_INF;
    double min_value_F = DBL_INF;
    double max_value_NL = -DBL_INF;
    double min_value_NL = DBL_INF;

    double arrayToFile_Fx[size_F];
    double arrayToFile_Fy[size_F];
    double arrayToFile_NLx[size_NL];
    double arrayToFile_NLy[size_NL];

    for (int k=0;k<size_F;k++){

        if (k < int(rangeToPlot/simStep)){
            (*LNPlot_F)(k,0,0,0) = F[2*k+1];

            arrayToFile_Fy[k] = (*LNPlot_F)(k,0,0,0);
            arrayToFile_Fx[k] = (rangeToPlot/size_F)*k;

            // Maximum and minimum used for normalization
            if ((*LNPlot_F)(k,0,0,0) > max_value_F)
                max_value_F = (*LNPlot_F)(k,0,0,0);
            if ((*LNPlot_F)(k,0,0,0) < min_value_F)
                min_value_F = (*LNPlot_F)(k,0,0,0);
        }
    }

    for (int k=0;k<size_NL;k++){

        (*LNPlot_NL)(k,0,0,0) = histogram_interpolated[k];

        arrayToFile_NLy[k] = (*LNPlot_NL)(k,0,0,0);
        arrayToFile_NLx[k] = min_g + (discard_bins/number_bins)*(max_g - min_g) +
                k*(max_g - min_g -2*(discard_bins/number_bins)*(max_g - min_g))/size_NL;

        // Maximum and minimum used for normalization
        if ((*LNPlot_NL)(k,0,0,0) > max_value_NL)
            max_value_NL = (*LNPlot_NL)(k,0,0,0);
        if ((*LNPlot_NL)(k,0,0,0) < min_value_NL)
            min_value_NL = (*LNPlot_NL)(k,0,0,0);
    }

    // Save results to file
    saveArray(arrayToFile_Fy,size_F,fileID+"Fy");
    saveArray(arrayToFile_Fx,size_F,fileID+"Fx");
    saveArray(arrayToFile_NLy,size_NL,fileID+"NLy");
    saveArray(arrayToFile_NLx,size_NL,fileID+"NLx");

    // Show displays
    if(showDisplay){
        CImg <unsigned char> *F_curve = new CImg <unsigned char>(400,256,1,3,0);
        CImg <unsigned char> *nonlinearity = new CImg <unsigned char>(400,256,1,3,0);
        F_curve->fill(*backColor);
        nonlinearity->fill(*backColor);

        drawDisp->assign((*F_curve,*nonlinearity),"LN analysis averaged");

        F_curve->draw_graph((LNPlot_F->get_crop(0,0,0,0,size_F-1,0,0,0)-min_value_F)*255/(max_value_F-min_value_F)
                            ,color1,1,1,4,255,0);
        nonlinearity->draw_graph((LNPlot_NL->get_crop(0,0,0,0,size_NL-1,0,0,0)-min_value_NL)*255/(max_value_NL-min_value_NL)
                            ,color1,1,1,4,255,0);

        // axes
        F_curve->draw_text(300,200,"Time (ms)",color2,backColor,1,20).display(*drawDisp);
        F_curve->draw_text(130,5,"Linear Filter",color2,backColor,1,20).display(*drawDisp);
        nonlinearity->draw_text(320,200,"Input",color2,backColor,1,20).display(*drawDisp);
        nonlinearity->draw_text(130,5,"Static NonLinearity",color2,backColor,1,20).display(*drawDisp);

        CImg <double> *x_axis1 = new CImg <double>(3,1,1,1,0);
        (*x_axis1)(0,0,0,0) = 0;
        (*x_axis1)(1,0,0,0) = rangeToPlot/2;
        (*x_axis1)(2,0,0,0) = rangeToPlot;

        CImg <double> *y_axis1 = new CImg <double>(1,3,1,1,0);
        (*y_axis1)(0,0,0,0) = max_value_F;
        (*y_axis1)(0,1,0,0) = (max_value_F-min_value_F)/2 + min_value_F;
        (*y_axis1)(0,2,0,0) = min_value_F;

        CImg <double> *x_axis2 = new CImg <double>(3,1,1,1,0);
        (*x_axis2)(0,0,0,0) = min_g + discard_bins*(max_g - min_g)/number_bins;
        (*x_axis2)(1,0,0,0) = (max_g - min_g)/2 + min_g;
        (*x_axis2)(2,0,0,0) = max_g - discard_bins*(max_g - min_g)/number_bins;

        CImg <double> *y_axis2 = new CImg <double>(1,3,1,1,0);
        (*y_axis2)(0,0,0,0) = max_value_NL;
        (*y_axis2)(0,1,0,0) = (max_value_NL-min_value_NL)/2 + min_value_NL;
        (*y_axis2)(0,2,0,0) = min_value_NL;

        (F_curve->draw_axis(*x_axis1,226,color2,1,~0U,20,true),
                nonlinearity->draw_axis(*x_axis2,226,color2,1,~0U,20,true)).display(*drawDisp);

        (F_curve->draw_axis(110,*y_axis1,color2,1,~0U,20,true),
                nonlinearity->draw_axis(110,*y_axis2,color2,1,~0U,20,true)).display(*drawDisp);

        // move display
        drawDisp->move(col,row);

        if(lastWindow){
            cout << "\rClose spatial multimeter window to continue..." << endl;
            while (!drawDisp->is_closed())
                drawDisp->wait();
        }
        delete nonlinearity;
        delete F_curve;
    }
    delete LNPlot_F;
    delete LNPlot_NL;

    // free memory
    free(F);
    free(denominator);
    free(numerator);
    free(S_conj);
    free(S);
    free(R);

}

void multimeter::fft(double data[], int nn, int isign){
    /*
     FFT/IFFT routine. (see pages 507-508 of Numerical Recipes in C)

     Inputs:
        data[] : array of complex* data points of size 2*NFFT+1.
            data[0] is unused,
            * the n'th complex number x(n), for 0 <= n <= length(x)-1, is stored as:
                data[2*n+1] = real(x(n))
                data[2*n+2] = imag(x(n))
            if length(Nx) < NFFT, the remainder of the array must be padded with zeros

        nn : FFT order NFFT. This MUST be a power of 2 and >= length(x).
        isign:  if set to 1,
                    computes the forward FFT
                if set to -1,
                    computes Inverse FFT - in this case the output values have
                    to be manually normalized by multiplying with 1/NFFT.
     Outputs:
        data[] : The FFT or IFFT results are stored in data, overwriting the input.
    */

    int n, mmax, m, j, istep, i;
    double wtemp, wr, wpr, wpi, wi, theta;
    double tempr, tempi;

    n = nn << 1;
    j = 1;
    for (i = 1; i < n; i += 2) {
    if (j > i) {
        tempr = data[j];     data[j] = data[i];     data[i] = tempr;
        tempr = data[j+1]; data[j+1] = data[i+1]; data[i+1] = tempr;
    }
    m = n >> 1;
    while (m >= 2 && j > m) {
        j -= m;
        m >>= 1;
    }
    j += m;
    }
    mmax = 2;
    while (n > mmax) {
    istep = 2*mmax;
    theta = TWOPI/(isign*mmax);
    wtemp = sin(0.5*theta);
    wpr = -2.0*wtemp*wtemp;
    wpi = sin(theta);
    wr = 1.0;
    wi = 0.0;
    for (m = 1; m < mmax; m += 2) {
        for (i = m; i <= n; i += istep) {
        j =i + mmax;
        tempr = wr*data[j]   - wi*data[j+1];
        tempi = wr*data[j+1] + wi*data[j];
        data[j]   = data[i]   - tempr;
        data[j+1] = data[i+1] - tempi;
        data[i] += tempr;
        data[i+1] += tempi;
        }
        wr = (wtemp = wr)*wpr - wi*wpi + wr;
        wi = wi*wpr + wtemp*wpi + wi;
    }
    mmax = istep;
    }

}

void multimeter::conj(double data[], double copy[], int NFFT){

    for(int i=0; i<NFFT; i++)
    {
        copy[2*i+1] = data[2*i+1];
        copy[2*i+2] = -data[2*i+2];
    }

}
