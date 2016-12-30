#include "constants.h"

namespace constants{
    extern const double outputfactor = 50.0; // to scale LN analysis

    // root path
    std::string getPath(){

        char* cwd;
        char buff[PATH_MAX + 1];

        cwd = getcwd( buff, PATH_MAX + 1 );
        std::string currentDir(cwd);
        std::string currentDirRoot = currentDir+"/";

        return currentDirRoot;
    }

}


