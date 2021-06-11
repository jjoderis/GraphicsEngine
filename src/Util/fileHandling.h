#ifndef ENGINE_UTIL_FILEHANDLING
#define ENGINE_UTIL_FILEHANDLING

#include <filesystem>
#include <string>
#include <vector>

namespace Util
{

// source: http://insanecoding.blogspot.com/2011/11/how-to-read-in-file-in-c.html
std::string readTextFile(const char *filePath);
void writeTextToFile(const char *filePath, const std::string &data);

std::vector<std::filesystem::path> getFilePaths(const char *directoryPath);
std::vector<std::filesystem::path> getDirectories(const char *directoryPath);
} // namespace Util

#endif