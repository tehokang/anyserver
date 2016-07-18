#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__

#include <string>
#include <iostream>
#include <fstream>
using namespace std;

class FileSystem
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
    FileSystem();
    ~FileSystem();
};



#endif
