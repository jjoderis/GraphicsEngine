#ifndef APPS_MODELER_UTIL_SCENELOADER_SERIALIZATION
#define APPS_MODELER_UTIL_SCENELOADER_SERIALIZATION

#include <filesystem>
#include <map>

#include <json.hpp>

namespace Engine
{
namespace Util
{
class OpenGLTextureIndex;
}

class Registry;
} // namespace Engine

namespace SceneUtil
{

nlohmann::json serializeScene(Engine::Registry &registry,
                              const std::filesystem::path &path,
                              const Engine::Util::OpenGLTextureIndex &textureIndex);

} // namespace SceneUtil

#endif