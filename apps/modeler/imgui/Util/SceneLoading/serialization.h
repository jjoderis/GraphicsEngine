#ifndef APPS_MODELER_UTIL_SCENELOADER_SERIALIZATION
#define APPS_MODELER_UTIL_SCENELOADER_SERIALIZATION

#include <filesystem>
#include <map>

#include <json.hpp>

namespace Engine
{
class Registry;
} // namespace Engine

namespace SceneUtil
{

nlohmann::json serializeScene(Engine::Registry &registry, const std::filesystem::path &path);

} // namespace SceneUtil

#endif