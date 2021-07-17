#ifndef APPS_MODELER_UTIL_SCENELOADER
#define APPS_MODELER_UTIL_SCENELOADER

#include <filesystem>

namespace Engine
{
class Registry;

namespace Util
{
void loadScene(const std::filesystem::path &path, Registry &registry);
void saveScene(const std::filesystem::path &path, Registry &registry);
} // namespace Util

} // namespace Engine

#endif