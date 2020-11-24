#ifndef ENGINE_UTIL_FILEHANDLING
#define ENGINE_UTIL_FILEHANDLING

#include <cstring>
#include <fstream>
#include <string>
#include <cerrno>
#include <filesystem>
#include <vector>
#include <iostream>

namespace Util {
    using Path = std::filesystem::path;

    // source: http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
    std::string readTextFile(const char* filePath);

    std::vector<Path> getFilePaths(const char* directoryPaths);
}

#endif