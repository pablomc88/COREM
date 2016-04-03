#include "constants.h"

namespace constants{
    extern std::string retinaFolder = "COREM";
    extern std::string retinaScript = "Retina_scripts/example_1.py";
    extern const char* resultID = "contrast";
    extern const double outputfactor = 50.0; // to scale LN analysis

    // root path
    std::string getPath(){

        char* cwd;
        char buff[PATH_MAX + 1];

        cwd = getcwd( buff, PATH_MAX + 1 );
        std::string currentDir(cwd);

        size_t pos = currentDir.find(constants::retinaFolder);
        std::string currentDirRoot = currentDir.substr(0,pos+(constants::retinaFolder.length())+1)+"/";

        if (pos==std::string::npos)
            std::cout << "error: folder of COREM simulator not found" << std::endl;

        return currentDirRoot;
    }

}


