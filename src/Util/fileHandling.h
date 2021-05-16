#ifndef ENGINE_UTIL_FILEHANDLING
#define ENGINE_UTIL_FILEHANDLING

#include <cerrno>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace Util
{

// source: http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
std::string readTextFile(const char *filePath);
void writeTextToFile(const char *filePath, const std::string &data);

std::vector<fs::path> getFilePaths(const char *directoryPath);
std::vector<fs::path> getDirectories(const char *directoryPath);
} // namespace Util

#endif