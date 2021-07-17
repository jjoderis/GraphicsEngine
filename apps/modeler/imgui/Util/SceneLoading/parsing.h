#ifndef APPS_MODELER_UTIL_SCENELOADER_PARSING
#define APPS_MODELER_UTIL_SCENELOADER_PARSING

#include <filesystem>
#include <json.hpp>
#include <map>

namespace Engine
{
class Registry;
} // namespace Engine

namespace SceneUtil
{

void parseScene(Engine::Registry &registry, nlohmann::json &j, const std::filesystem::path &path);

} // namespace SceneUtil

#endif