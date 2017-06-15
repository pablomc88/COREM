#ifndef MULTIMETER_H
#define MULTIMETER_H

/* BeginDocumentation
 * Name: multimeter
 *
 * Description: A multimeter records the output from connected modules to file.
 * There are 3 types of multimeters: temporal, spatial or linear-nonlinear (LN)
 * analysis [1]. Results are saved to the folder "results". Every multimeter
 * generates a file with its identifier.
 *
 * [1] Baccus, Stephen A., and Markus Meister. "Fast and slow contrast adaptation
 * in retinal circuitry." Neuron 36.5 (2002): 909-919
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 */

#include "../CImg-1.6.0_rolling141127/CImg.h"
#include "vector"
#include "string"
#include<iostream>
#include <fstream>
#include <numeric>

#include "constants.h"

using namespace cimg_library;
using namespace std;

class multimeter{
protected:
    // Image size
    int sizeX, sizeY;
    // simulation step
    double simStep;
    // Image display for the multimeter (windows destroyed when object deleted)
    CImgDisplay *drawDisp;
    // to save temporal evolution of cell dynamics (used for the time plot)
    vector < vector <double> > timeRecord;
    // to save values of the input stimulus (used for the time plot)
    vector <double> input;
    // similarly for the LN analysis
    vector < vector <double> > LN_input;
    vector < vector <double> > LN_timeRecord;
    // if False, only the output of one selected cell is saved to timeRecord
    bool recordAllCells;
    // start time for time plots
    double startTime;
    // Time range to plot the F filter for the LN analysis
    double rangeToPlot;

public:
    // Constructor, copy, destructor.
    multimeter(int x=1,int y=1, double step=1);
    multimeter(const multimeter& copy);
    ~multimeter(void);

    // Set methods
    void setSizeX(int x);
    void setSizeY(int y);
    void setStep(double step);
    void setRecordAllCells(bool value);
    void setStartTime(double value);
    void setRangeToPlot(double value);

    // Get methods
    bool getRecordAllCells();

    // Initialize timeRecord
    void initializeTimeRecord();

    // Initialize vectors used for the LN analysis
    void initializeLNAnalysis(int numberTrials);

    // Save cell's output values to timeRecord every simulation step
    void recordValue(double value,int cell);

    // Save values of the input stimulus
    void recordInput(double value);

    // similarly for the LN analysis
    void recordValueLNAnalysis(double value,int trial);
    void recordInputLNAnalysis(double value,int trial);

    // Get path to the working directory
    string getWorkingDir();

    // Write the content of an array into text file
    void saveArray(double array[], int size, string fileID);

    // Load from text file
    vector <double> loadArray(string fileID);

    // Save all vectors to file (used in multiple-trial simulations if the
    // multimeter object is destroyed)
    void saveAllVectors(int trial);

    // Load all vectors from file
    void loadAllVectors(int numberTrials);

    // Spatial multimeter
    void showSpatialProfile(CImg<double> *img, bool rowCol, int cell, string title, int col, int row,
                            bool lastWindow, bool showDisplay, string fileID);

    // Time multimeter
    void showTimeProfile(string title, int col, int row, bool lastWindow,
                                        bool showDisplay, string fileID);

    // LN analysis
    void showLNAnalysis(int col, int row, bool lastWindow,
                        bool showDisplay, string fileID, double segment, double interval,
                        double start, double stop, int numberTrials);

    // FFT transform (for the LN analysis)
    void fft(double data[], int nn, int isign);
    void conj(double data[], double copy[], int NFFT);

};

#endif // MULTIMETER_H
