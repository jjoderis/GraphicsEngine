#include "fileHandling.h"

std::string Util::readTextFile(const char *filePath)
{
    std::ifstream in(filePath, std::ios::in | std::ios::binary);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return (contents);
    }
    throw(errno);
}

void Util::writeTextToFile(const char *filePath, const std::string &data)
{
    std::ofstream out{filePath, std::ios::out | std::ios::binary};
    if (out)
    {
        out << data;
        out.close();
    }
    else
    {
        throw(errno);
    }
}

std::vector<fs::path> Util::getFilePaths(const char *directoryPath)
{
    std::vector<fs::path> filePaths{};

    for (const auto &entry : std::filesystem::directory_iterator(directoryPath))
    {
        if (entry.is_regular_file())
        {
            filePaths.push_back(entry.path());
        }
    }

    return filePaths;
}

std::vector<fs::path> Util::getDirectories(const char *directoryPath)
{
    std::vector<fs::path> directoryPaths{};

    for (const auto &entry : std::filesystem::directory_iterator(directoryPath))
    {
        if (entry.is_directory())
        {
            directoryPaths.push_back(entry.path());
        }
    }

    return directoryPaths;
}