#ifndef APPS_MODELER_UTIL_OBJECTLOADER
#define APPS_MODELER_UTIL_OBJECTLOADER

#include <filesystem>
namespace Engine
{
class Registry;
}
namespace Util
{
void loadOBJFile(Engine::Registry &registry, const std::filesystem::path &filePath);
}

#endif