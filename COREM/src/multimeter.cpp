#include <algorithm> // for std::min
#include <cstring> // for size_t
#include "multimeter.h"

multimeter::multimeter(int x, int y){
    sizeX = x;
    sizeY = y;
    simStep=1.0;
    draw_disp = new CImgDisplay();
}

multimeter::multimeter(const multimeter& copy){
    sizeX=copy.sizeX;
    sizeY=copy.sizeY;
    simStep=copy.simStep;
    draw_disp = new CImgDisplay(*copy.draw_disp);
}

multimeter::~multimeter(){
    delete draw_disp;
}

void multimeter::setSimStep(double value){
    simStep = value;
}

//------------------------------------------------------------------------------//

void multimeter::showSpatialProfile(CImg<double> *img,bool rowCol,int number,string title,int col,int row,double waitTime){

    double dim = 0.0;

    if(rowCol == true){
        dim = img->width();
    }else{
        dim = img->height();
    }

    CImg <double> *SpatialProfile1 = new CImg <double>(dim,1,1,1,0);

    double max_value1 = DBL_EPSILON;
    double min_value1 = DBL_INF;

    for(int k=0;k<dim;k++){
        if(rowCol == true){
            (*SpatialProfile1)(k,0,0,0)=(*img)(k,number,0,0);
        }else{
            (*SpatialProfile1)(k,0,0,0)=(*img)(number,k,0,0);
        }

        if ((*SpatialProfile1)(k,0,0,0)>max_value1)
            max_value1 = (*SpatialProfile1)(k,0,0,0);
        if ((*SpatialProfile1)(k,0,0,0)<min_value1)
            min_value1 = (*SpatialProfile1)(k,0,0,0);

    }

    if(max_value1==min_value1){
        max_value1+=1;
        min_value1-=1;
    }

    if(min_value1>0)
        min_value1 = 0;
    if(max_value1<0)
        max_value1 = 0;

//    // plot
    if(waitTime > -2){

        const unsigned char color1[] = {255,0,0};
        const unsigned char color2[] = {0,0,255};
        unsigned char back_color[] = {255,255,255};
        CImg <unsigned char> *profile = new CImg <unsigned char>(400,256,1,3,0);
        profile->fill(*back_color);
        const char * titleChar = (title).c_str();
        draw_disp->assign(*profile,titleChar);

        profile->draw_graph(SpatialProfile1->get_crop(0,0,0,0,dim-1,0,0,0)*255/(max_value1-min_value1) - min_value1*255/(max_value1-min_value1),color1,1,1,4,255,0).display(*draw_disp);
        profile->draw_axes(0.0,dim,max_value1,min_value1,color2,1,-80,-80,0,0,~0U,~0U,20).display(*draw_disp);
        profile->draw_text(320,200,"pixels",color2,back_color,1,20).display(*draw_disp);
        profile->draw_text(40,5,"Output",color2,back_color,1,20).display(*draw_disp);

        // move display
        draw_disp->move(col,row);

        if(waitTime == -1){
            cout << "\rClose spatial multimeter window to continue..." << endl;
            while (!draw_disp->is_closed())
                draw_disp->wait();
        }
        delete profile;
    }

    delete SpatialProfile1;
}

//------------------------------------------------------------------------------//

void multimeter::recordValue(double value){
    temporal.push_back(value);
}

void multimeter::recordInput(double value){
    input.push_back(value);
}

//------------------------------------------------------------------------------//


void multimeter::showTemporalProfile(string title,int col,int row, double waitTime, const char * TempFile){


    CImg <double> *temporalProfilet = new CImg <double>(temporal.size(),1,1,1,0);
    double temp[temporal.size()];

    double max_value = DBL_EPSILON;
    double min_value = DBL_INF;

    for(size_t k=0;k<temporal.size();k++){
        (*temporalProfilet)(k,0,0,0)=temporal[k];
        temp[k] = temporal[k];

        if (temporal[k]>max_value)
            max_value = temporal[k];
        if (temporal[k]<min_value)
            min_value = temporal[k];
    }

    if(max_value==min_value){
        max_value+=1;
        min_value-=1;
    }
/*
    if(min_value>0)
        min_value = 0;
    if(max_value<0)
        max_value = 0;
*/
    // remove file and save new file
    removeFile(TempFile);
    saveArray(temp,temporal.size(),TempFile);

    // plot
    if(waitTime > -2){

        const unsigned char color1[] = {255,0,0};
        const unsigned char color2[] = {0,0,255};
        unsigned char back_color[] = {255,255,255};
        CImg <unsigned char> *profile = new CImg <unsigned char>(400,256,1,3,0);
        profile->fill(*back_color);
        const char * titleChar = (title).c_str();
        draw_disp->assign(*profile,titleChar); // // This object must not be deleted (it is static) in order to allow several temporal multimeter to remain open simultaneously

        profile->draw_graph(temporalProfilet->get_crop(0,0,0,0,temporal.size()-1,0,0,0)*255/(max_value-min_value) - min_value*255/(max_value-min_value),color1,1,1,1,255,0).display(*draw_disp);
        profile->draw_axes(0.0,temporal.size()*simStep,max_value,min_value,color2,1,-80,-80,0,0,~0U,~0U,20).display(*draw_disp);
        profile->draw_text(320,200,"time (ms)",color2,back_color,1,20).display(*draw_disp);
        profile->draw_text(40,5,"Output",color2,back_color,1,20).display(*draw_disp);

        // move display
        draw_disp->move(col,row);

        if(waitTime == -1){
            cout << "\rClose last temporal multimeter window to continue..." << endl;
            while (!draw_disp->is_closed())
                draw_disp->wait();
        }
        
        delete profile;
    }
    delete temporalProfilet;
}

//------------------------------------------------------------------------------//

void multimeter::showLNAnalysisAvg(int col, int row, double waitTime,double segment, double start, double stop, double numberTrials, const char * LNFile, double ampl){

    // normalize input
    double mean_value1 = 0;

    for(size_t k=0;k<input.size();k++){
        mean_value1+= input[k];
    }

    mean_value1 /= input.size();

    for(size_t k=0;k<input.size();k++){
        input[k] = (input[k] - mean_value1);
    }

    // read values from file
    string sto_file = composeResultsPath(LNFile);
	const char* to_file = sto_file.c_str();    
	vector<double> F;

    std::ifstream fin;
    fin.open(to_file, std::ifstream::in);
    if(fin.is_open())
    {

        while (!fin.eof())
        {
          char buf[1000];
          fin.getline(buf,1000);
          const char* token[1] = {};
          token[0] = strtok(buf, "\n");
          if(token[0] != NULL)
             F.push_back(atof(token[0]));

        }

        fin.close(); 
    }
    // Non-linearity: The stimulus is convolved with the filter.
    // g = S*F

//    int length = (stop-start);

    const char * seq1 = "inp";
    const char * seq2 = "tmp";

    char seqFile1[1000];
    char seqFile2[1000];

    strcpy(seqFile1,seq1);
    strcat(seqFile1,LNFile);

    strcpy(seqFile2,seq2);
    strcat(seqFile2,LNFile);

    vector <double> historyInput = readSeq(seqFile1);
    vector <double> historyTemporal = readSeq(seqFile2);
    int length = historyInput.size();

    // create arrays to store FFT of G, S and F

    int NFFT = (int)pow(2.0, ceil(log((double)segment)/log(2.0)));
    int newNFFT = (int)pow(2.0, ceil(log((double)length)/log(2.0)));

    double *g = (double *) malloc((2*newNFFT+1) * sizeof(double));
    double *newS = (double *) malloc((2*newNFFT+1) * sizeof(double));
    double *newF = (double *) malloc((2*newNFFT+1) * sizeof(double));

    g[0]=0.0;
    newS[0]=0.0;
    newF[0]=0.0;

//    for(int i=(int)start; i<length+start; i++)
//    {
//        newS[2*(i-(int)start)+1] = input[i];
//        newS[2*(i-(int)start)+2] = 0.0;

//        g[2*(i-(int)start)+1] = 0.0;
//        g[2*(i-(int)start)+2] = 0.0;

//    }

//    for(int i=length+start; i<newNFFT+(int)start; i++)
//    {
//        newS[2*(i-(int)start)+1] = 0.0;
//        newS[2*(i-(int)start)+2] = 0.0;

//        g[2*(i-(int)start)+1] = 0.0;
//        g[2*(i-(int)start)+2] = 0.0;
//    }

    for(int i=0; i<length; i++)
    {
        newS[2*i+1] = historyInput[i];
        newS[2*i+2] = 0.0;

        g[2*i+1] = 0.0;
        g[2*i+2] = 0.0;
    }

    for(int i=length; i<newNFFT; i++)
    {
        newS[2*i+1] = 0.0;
        newS[2*i+2] = 0.0;

        g[2*i+1] = 0.0;
        g[2*i+2] = 0.0;
    }

    for(int i=0; i<newNFFT; i++)
    {
        if(i<NFFT){
            newF[2*i+1]=(F[2*i+1]/numberTrials);
            newF[2*i+2]=(F[2*i+2]/numberTrials);
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
        g[2*i+1] = (newF[2*i+1]*newS[2*i+1]) - (newF[2*i+2]*newS[2*i+2]);
        g[2*i+2] = (newF[2*i+1]*newS[2*i+2]) + (newF[2*i+2]*newS[2*i+1]);
    }

    // iFFT of g
    fft(g, newNFFT, -1);

    // normalize the IFFT of g
    for(int i=0; i<newNFFT; i++)
    {
        g[2*i+1] /= newNFFT;
        g[2*i+2] /= newNFFT;

    }

    // Normalize g: the variance of the filtered stimulus is equal to
    // the variance of the stimulus (Baccus 2002)

    double varianceG = 0;
    double varianceS = 0;

//    for (int k=0;k<newNFFT;k++) {
//        if((k+start)<(start+length))
//            varianceS += input[k+start]*input[k+start];
//         varianceG += g[2*k+1]*g[2*k+1];
//    }

    for (int k=0;k<newNFFT;k++) {
        if(k<length)
            varianceS += historyInput[k]*historyInput[k];
        varianceG += g[2*k+1]*g[2*k+1];
    }

    varianceS /= length;
    varianceG /= newNFFT;

    for(int k=0;k<NFFT;k++)
        F[2*k+1]*=(sqrt(varianceS)/sqrt(varianceG))*ampl;

    for(int k=0;k<newNFFT;k++)
        g[2*k+1]*=(sqrt(varianceS)/sqrt(varianceG));

    // Plot of F
    segment = int(segment/3);
    CImg <double> *temporalProfile = new CImg <double>(segment,1,1,1,0);

    double max_value1 = 0;
    double min_value1 = 0;

    double max_valuex = 0;double max_valuey = -DBL_INF;
    double min_valuex = 0;double min_valuey = DBL_INF;

    double auxF[int(segment)];

    for(int k=0;k<segment;k++){
        (*temporalProfile)(k,0,0,0)=F[2*k+1]/numberTrials;
        auxF[k]=F[2*k+1]/numberTrials;
        if (F[2*k+1]/numberTrials>max_value1)
            max_value1 = F[2*k+1]/numberTrials;
        if (F[2*k+1]/numberTrials<min_value1)
            min_value1 = F[2*k+1]/numberTrials;
    }

    if(min_value1 == max_value1){ // Prevent min and max from being equal since values are later normalized to max-min
        min_value1-=0.5;
        max_value1+=0.5;
    }

    // remove LN file and save new file
    removeFile(LNFile);
    saveArray(auxF,int(segment),LNFile);

    // discard the beginning of the sequence to plot g
    //int begin = start;
    //if(length > 300)
    //    begin = start+200;

    // Plot of the response vs g

    max_valuex = sqrt(varianceS);
    min_valuex = -sqrt(varianceS);

    CImg <double> *staticProfile = new CImg <double>(400,1,1,1,0);
    double numberPoints[400];for(int l=0;l<400;l++)numberPoints[l]=0;


//    for(int k=begin;k<(start+length);k++){
//        if(abs(g[2*(k-(int)start)+1]) < sqrt(varianceS)){
//            double value1 = (g[2*(k-(int)start)+1]-min_valuex)*(399/(max_valuex-min_valuex));
//            (*staticProfile)(int(value1),0,0,0)+=(k/((start+length)))*temporal[k];
//            numberPoints[int(value1)]+=(k/((start+length)));
////            (*staticProfile)(int(value1),0,0,0)=temporal[k];
//        }
//    }

    for(int k=0;k<length;k++){
        if(abs(g[2*k+1]) < sqrt(varianceS)){
            double value1 = (g[2*k+1]-min_valuex)*(399/(max_valuex-min_valuex));
            (*staticProfile)(int(value1),0,0,0)+=historyTemporal[k];
            numberPoints[int(value1)]+=1;
//            (*staticProfile)(int(value1),0,0,0)=temporal[k];
        }
    }

    free(newF);
    free(newS);
    free(g);
    
    double auxSP[400];
    double auxSP2[400];

    for(int l=0;l<400;l++){

        if(numberPoints[l]>0)
            (*staticProfile)(l,0,0,0)/=numberPoints[l];
        auxSP[l] = (*staticProfile)(l,0,0,0);

        if((*staticProfile)(l,0,0,0)<min_valuey && abs((*staticProfile)(l,0,0,0)) > 0)
            min_valuey = (*staticProfile)(l,0,0,0);
        if((*staticProfile)(l,0,0,0)>max_valuey && abs((*staticProfile)(l,0,0,0)) > 0)
            max_valuey = (*staticProfile)(l,0,0,0);
    }

    if(min_valuey == max_valuey){ // Prevent min and max from being equal
        min_valuey-=0.5;
        max_valuey+=0.5;
    }

    (*staticProfile)(399,0,0,0) = (*staticProfile)(398,0,0,0);

    for(int l=0;l<400;l++){
        auxSP2[l] = (l-200)*((max_valuex-min_valuex)/399);
    }


    // Simple average of points
    int bin_size = 100;// even number
    CImg <double> *newStaticProfile = new CImg <double>(400,1,1,1,0);

    for(int l=0;l<400;l++){
        double accumulator = 0;

        if(l>=bin_size/2 && l<400-bin_size/2){
            for(int k=1;k<bin_size/2+1;k++)
                accumulator+= (*staticProfile)(l-k,0,0,0);
            for(int k=0;k<bin_size/2;k++)
                accumulator+= (*staticProfile)(l+k,0,0,0);

            (*newStaticProfile)(l,0,0,0) = accumulator/bin_size;
        }
        else if(l<bin_size/2){
            for(int k=0;k<l+bin_size/2;k++)
                accumulator+= (*staticProfile)(k,0,0,0);

            (*newStaticProfile)(l,0,0,0) = accumulator/(l+bin_size/2);
        }else{
            for(int k=l-bin_size/2;k<400;k++)
                accumulator+= (*staticProfile)(k,0,0,0);

            (*newStaticProfile)(l,0,0,0) = accumulator/(400-l+bin_size/2);
        }



    }

    for(int l=0;l<400;l++){
        (*staticProfile)(l,0,0,0) = (*newStaticProfile)(l,0,0,0);
        auxSP[l] = (*staticProfile)(l,0,0,0);
    }

    delete newStaticProfile;


    // Remove and save new static NL
    const char * SNL = "SNL";
    const char * SNL2 = "SNL2";
    char f1[1000];
    char f2[1000];
    strcpy(f1,SNL);
    strcat(f1,LNFile);
    strcpy(f2,SNL2);
    strcat(f2,LNFile);

    removeFile((const char*)f1);
    removeFile((const char*)f2);
    saveArray(auxSP,400,f1);
    saveArray(auxSP2,400,f2);


    // display results

    // code for parameter waitTime:
    // waitTime = -2 -> display is not shown


    if(waitTime > -2){

        const unsigned char color1[] = {255,0,0};
        const unsigned char color2[] = {0,0,255};
        unsigned char back_color[] = {255,255,255};
        CImg <unsigned char> *profile = new CImg <unsigned char>(400,256,1,3,0);
        CImg <unsigned char> *nonlinearity = new CImg <unsigned char>(400,256,1,3,0);
        profile->fill(*back_color);
        nonlinearity->fill(*back_color);

        draw_disp->assign((*profile,*nonlinearity),"LN analysis averaged"); // Window is closed when this object is deleted
        profile->draw_graph(temporalProfile->get_crop(0,0,0,0,segment-1,0,0,0)*255/(max_value1-min_value1) - min_value1*255/(max_value1-min_value1),color1,1,1,4,255,0);
        nonlinearity->draw_graph(staticProfile->get_crop(0,0,0,0,400-1,0,0,0)*255/(max_valuey-min_valuey) - min_valuey*255/(max_valuey-min_valuey),color1,1,1,4,255,0);
        profile->draw_text(320,200,"time (ms)",color2,back_color,1,20).display(*draw_disp);
        profile->draw_text(40,5,"Linear Filter",color2,back_color,1,20).display(*draw_disp);
        nonlinearity->draw_text(320,200,"Input",color2,back_color,1,20).display(*draw_disp);
        nonlinearity->draw_text(10,5,"Static NonLinearity",color2,back_color,1,20).display(*draw_disp);

        (profile->draw_axes(0,segment*simStep,max_value1,min_value1,color2,1,-80,-80,0,0,~0U,~0U,20),nonlinearity->draw_axes(min_valuex,max_valuex,max_valuey,min_valuey,color2,1,-40,-20,0,0,~0U,~0U,13)).display(*draw_disp);


        // move display
        draw_disp->move(col,row);

        // wait for the user closes display
        cout << "\rClose LN analysis multimeter window to continue..." << endl;
        while (!draw_disp->is_closed())
            draw_disp->wait();

        delete nonlinearity;
        delete profile;
    }

    delete temporalProfile;
    delete staticProfile;    
}

//------------------------------------------------------------------------------//

void multimeter::showLNAnalysis(string title, int col, int row, double waitTime, double segment, double interval, double start, double stop,double numberTrials,const char * LNFile){


    // normalize vectors
    vector <double> newInput;
    vector <double> newTemporal;
    vector <double> historyInput;
    vector <double> historyTemporal;

    double mean_value1 = 0;
    double mean_value2 = 0;

    for(size_t k=0;k<input.size();k++){
        mean_value1+= input[k];
        mean_value2+= temporal[k];
    }

    mean_value1 /= input.size();
    mean_value2 /= temporal.size();

    for(size_t k=0;k<input.size();k++){
        if(k>temporal.size()/100){
            newInput.push_back(input[k] - mean_value1);
    //        input[k] = (input[k] - mean_value1);
            newTemporal.push_back(temporal[k] - mean_value2);
    //        temporal[k] = temporal[k] - mean_value2;
        }else{
            newInput.push_back(0.0);
    //        input[k] = 0.0;
            newTemporal.push_back(0.0);
    //        temporal[k] = 0.0;
        }

        if(k>=start && k<stop){
            historyInput.push_back(input[k] - mean_value1);
            historyTemporal.push_back(temporal[k]);
        }

    }

    // save seq
    const char * seq1 = "inp";
    const char * seq2 = "tmp";

    char seqFile1[1000];
    char seqFile2[1000];

    strcpy(seqFile1,seq1);
    strcat(seqFile1,LNFile);

    strcpy(seqFile2,seq2);
    strcat(seqFile2,LNFile);

    saveSeq(historyInput,seqFile1,(stop-start)*2*simStep);
    saveSeq(historyTemporal,seqFile2,(stop-start)*2*simStep);

    // calculate NFFT as the next higher power of 2 >= Nx.
    int NFFT = (int)pow(2.0, ceil(log((double)segment)/log(2.0)));

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

    int number_rep = int((stop-start)/interval);
    for (int rep=0;rep<number_rep;rep++){
        // Storing vectors in a complex array. This is needed even though x(n) is purely real in this case.
        for(int i=0; i<segment; i++)
        {
            R[2*i+1] = newTemporal[start + rep*interval + i];
            R[2*i+2] = 0.0;

            S[2*i+1] = newInput[start + rep*interval + i];
            S[2*i+2] = 0.0;

            S_conj[2*i+1] = 0.0;
            S_conj[2*i+2] = 0.0;
        }
        // pad the remainder of the array with zeros (0 + 0 j) //
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

            if(rep==0){
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


    for(int i=0; i<NFFT; i++)
    {

        numerator[2*i+1] /= number_rep;
        numerator[2*i+2] /= number_rep;
        denominator[2*i+1] /= number_rep;
        denominator[2*i+2] /= number_rep;

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


    saveArray(F,int(2*NFFT+1),LNFile);

    // code for parameter waitTime:
    // waitTime = -2 -> display is not shown
    // waitTime = -1 -> display is shown till the user close it
    // waitTime >= 0 -> display is shown a duration waitTime

    if(waitTime > -2 and numberTrials==1){

//        CImg <double> *temporalProfile = new CImg <double>(segment,1,1,1,0);

//        double max_value1 = 0;
//        double min_value1 = 0;


//        for(int k=0;k<segment;k++){
//            (*temporalProfile)(k,0,0,0)=F[2*k+1];

//            if (F[2*k+1]>max_value1)
//                max_value1 = F[2*k+1];
//            if (F[2*k+1]<min_value1)
//                min_value1 = F[2*k+1];
//        }


//        const unsigned char color1[] = {255,0,0};
//        const unsigned char color2[] = {0,0,255};
//        unsigned char back_color[] = {255,255,255};
//        CImg <unsigned char> *profile = new CImg <unsigned char>(400,256,1,3,0);
//        profile->fill(*back_color);
//        const char * titleChar = (title).c_str();
//        draw_disp->assign(*profile,titleChar);

//        profile->draw_graph(temporalProfile->get_crop(0,0,0,0,segment-1,0,0,0)*255/(max_value1-min_value1) - min_value1*255/(max_value1-min_value1),color1,1,1,4,255,0);
//        profile->draw_axes(0.0,segment*simStep,max_value1,min_value1,color2,1,-80,-80,0,0,~0U,~0U,20).display(*draw_disp);

//        // move display
//        draw_disp->move(col,row);

//        // wait for the user closes display
//        if(waitTime == -1){
//            while (!draw_disp->is_closed()) {
//                draw_disp->wait();
//            }
//        }

    }

    free(F);
    free(denominator);
    free(numerator);
    free(S_conj);
    free(S);
    free(R);
}

//------------------------------------------------------------------------------//

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

//------------------------------------------------------------------------------//


void multimeter::conj(double data[], double copy[], int NFFT){

    for(int i=0; i<NFFT; i++)
    {
        copy[2*i+1] = data[2*i+1];
        copy[2*i+2] = -data[2*i+2];
    }

}

//------------------------------------------------------------------------------//

string multimeter::getWorkingDir(){

    char* cwd;
    char buff[PATH_MAX + 1];

    cwd = getcwd( buff, PATH_MAX + 1 );
    string currentDir(cwd);

    //size_t pos = currentDir.find(constants::retinaFolder);
    //string currentDirRoot = currentDir.substr(0,pos+(constants::retinaFolder.length())+1)+"/";
    string currentDirRoot = currentDir+"/";

    //if (pos==std::string::npos)
    //    cout << "error: impossible to find the simulator folder" << endl;

    return currentDirRoot;
}

//------------------------------------------------------------------------------//

string multimeter::composeResultsPath(const char * File){

    string sFile = (string) File; 
    string stringResult = getWorkingDir()+ "results/" + sFile;

    return stringResult;
}

//------------------------------------------------------------------------------//


void multimeter::removeFile(const char *File){

    string stringResult = getWorkingDir()+ "results/";
    const char * root = (stringResult).c_str();
    char result[1000];

    strcpy(result,root);
    strcat(result,File);

    remove((const char*)result);
}

//------------------------------------------------------------------------------//

vector<double> multimeter::readSeq(const char * LNFile){

    // read data from file
    const char * seq = "seq";
    char seqFile[1000];
    strcpy(seqFile,seq);
    strcat(seqFile,LNFile);

    string sto_file = composeResultsPath((const char *)seqFile);
	const char* to_file = sto_file.c_str();
    std::ifstream fin;
    fin.open(to_file);
    vector<double> F;

    if (fin.good()) {
        while (!fin.eof())
        {
            char buf[1000];
            fin.getline(buf,1000);
            const char* token[1] = {};
            token[0] = strtok(buf, "\n");
            if(token[0] != NULL)
                F.push_back(atof(token[0]));
        }
        fin.close();
    }
    return F;
}

//------------------------------------------------------------------------------//


void multimeter::saveSeq(vector<double> newSeq, const char *LNFile, double maxSize){

    // read first
    vector<double> FileSeq = readSeq(LNFile);

    const char * seq = "seq";
    char seqFile[1000];
    strcpy(seqFile,seq);
    strcat(seqFile,LNFile);

    // new seq
    if(FileSeq.size()==0){

        // save new array
        double *F = (double *) malloc((newSeq.size()) * sizeof(double));
        if(F != NULL) {
            for(size_t k=0;k<newSeq.size();k++)
                F[k]=newSeq[k];
            saveArray(F, newSeq.size(), (const char *)seqFile);
            free(F);
        } else perror("saving new multimeter array");
    // another seq
    }else{

        if(FileSeq.size() < maxSize){
            // remove file
            removeFile((const char *)seqFile);

            // save new array
            double *F = (double *) malloc((FileSeq.size() + newSeq.size() ) * sizeof(double));
            if(F != NULL) {
                for(size_t k=0;k<FileSeq.size() + newSeq.size();k++){
                    if(k<FileSeq.size()){
                        F[k] = FileSeq[k];
                    }else{
                        F[k] = newSeq[k-FileSeq.size()];
                    }
                }

                saveArray(F, FileSeq.size() + newSeq.size(),(const char *)seqFile);
                free(F);
            } else perror("saving new multimeter array (another seq)");
        }
    }
}

//------------------------------------------------------------------------------//

void multimeter::saveArray(double* array, size_t arraySize, string fileID){

      // read file
      std::vector <std::string> files;
      dirent* de;

      string stringResult = getWorkingDir()+ "results/";
      const char * currentDir = (stringResult).c_str();
      DIR* dp=opendir(currentDir);

      if (dp){
          while (true)
          {
              de = readdir( dp );
              if (de == NULL)
                  break;
              files.push_back(std::string(de->d_name));
          }
        closedir( dp );
        std::sort( files.begin(), files.end() );
      }else{
          cout << "the directory \"results\" cannot be opened!" << endl;
      }

      bool fileFound = false;

      for(size_t k=0;k<files.size();k++){

          const char * f1 = (files[k]).c_str();
          const char * f2 = fileID.c_str();
          if (strcmp(f1,f2)==0){

              fileFound = true;
              // Read current value
              const char * fileName = (fileID).c_str();
              string sto_file = composeResultsPath(fileName);
				const char* to_file = sto_file.c_str();
              fin.precision(64);
              fin.open(to_file);
              if (fin.good()) {
                  vector <string> fileValues;

                  while (!fin.eof())
                  {
                    char buf[1000];
                    fin.getline(buf,1000);
                    const char* token[1] = {};
                    token[0] = strtok(buf, "\n");
                    if(token[0] != NULL) // If the last line ends with \n, we may have an extra loop iteration with token[0] == NULL
                        fileValues.push_back(token[0]);
                  }

                  fin.close();

                  // update values and write to file
                  fout.open(to_file);
                  if (fout.good()) {
                      const char * add_value;
                      for(size_t l=0;l<min(arraySize,fileValues.size());l++){
                          add_value = (fileValues[l]).c_str();
                          fout.precision(64);
                          if(l<arraySize-1)
                              fout << (array[l]+atof(add_value)) << endl;
                          else
                              fout << (array[l]+atof(add_value));
                      }
                      
                      fout.close();
                  }
              }
          }
      }

      // new file
      if(fileFound==false){

          // Read current value
          const char * fileName = (fileID).c_str();
          string sto_file = composeResultsPath(fileName);
const 		char* to_file = sto_file.c_str();
          // write new file
          fout.open(to_file);
          if (fout.good()) {
              for(size_t l=0;l<arraySize;l++){
                  fout.precision(64);
                  if(l<arraySize-1)
                      fout << array[l] << endl;
                  else
                      fout << array[l];
              }
              fout.close();
          }
      }
}

