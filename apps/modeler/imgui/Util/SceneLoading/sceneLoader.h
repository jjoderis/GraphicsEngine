#ifndef APPS_MODELER_UTIL_SCENELOADER
#define APPS_MODELER_UTIL_SCENELOADER

#include <filesystem>

namespace Engine
{
namespace Util
{
class OpenGLTextureIndex;
}

class Registry;

namespace Util
{
void loadScene(const std::filesystem::path &path, Registry &registry, Engine::Util::OpenGLTextureIndex &textureIndex);
void saveScene(const std::filesystem::path &path,
               Registry &registry,
               const Engine::Util::OpenGLTextureIndex &textureIndex);
} // namespace Util

} // namespace Engine

#endif