#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <string>
#include <iostream>
#include <fstream>
using namespace std;

class Utilities
{
public:
    static bool isExistFile(const string &file)
    {
        if ( file.empty() ) return false;
        /**
         * @note Do not close files, the file will be closed automatically.
         */
        std::ifstream filestream;
        filestream.open(file.c_str(), std::ios::in | std::ios::binary);
        return filestream.good();
    }

private:
    Utilities();
    ~Utilities();
};



#endif
