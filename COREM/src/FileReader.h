#ifndef FILEREADER_H
#define FILEREADER_H


/* BeginDocumentation
 * Name: FileReader
 *
 * Description: parsing of retina script and initialization of retina modules
 *
 * Author: Pablo Martinez Cañada. University of Granada. CITIC-UGR. Spain.
 * <pablomc@ugr.es>
 *
 */

#include "DisplayManager.h"
#include "StaticNonLinearity.h"
#include "ShortTermPlasticity.h"

using namespace cimg_library;
using namespace std;

// fixed parameters
const int MAX_CHARS_PER_LINE = 2048;
const int MAX_TOKENS_PER_LINE = 50;
const char* const DELIMITER1 = ".(),'";
const char* const DELIMITER2 = ",')";


class FileReader{
private:
    // to check syntax errors
    bool CorrectFile;
    bool continueReading;

    // File reader
    const char* fileName;
    ifstream fin;

public:

    //Constructor, copy, destructor.
    FileReader(int X=25, int Y=25, double tstep=1.0);
    FileReader(const FileReader& copy);
    ~FileReader(void);

    //reset parameters
    void reset(int X, int Y, double tstep);
    //set directory
    void setDir(const char* s);
    // allocate values
    void allocateValues();

    // parsing of retina script and initialization of retina modules
    void parseFile(Retina &retina,DisplayManager &displayMg);
    bool getContReading(){return continueReading;}
    void abort(int line);
};

#endif // FILEREADER_H
