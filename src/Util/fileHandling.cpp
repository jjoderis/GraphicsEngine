#include "fileHandling.h"

std::string Util::readTextFile(const char* filePath){
  std::ifstream in(filePath, std::ios::in | std::ios::binary);
  if (in)
  {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  throw(errno);
}

std::vector<Util::Path> Util::getFilePaths(const char* directoryPath) {
  std::vector<Path> filePaths{};

  for(const auto& entry: std::filesystem::directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
          filePaths.push_back(entry.path());
        }
  }

  return filePaths;
}