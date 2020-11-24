#ifndef ENGINE_UTIL_FILEHANDLING
#define ENGINE_UTIL_FILEHANDLING

#include <cstring>
#include <fstream>
#include <string>
#include <cerrno>

namespace Util {
    // source: http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
    std::string readTextFile(const char* filePath);
}

#endif