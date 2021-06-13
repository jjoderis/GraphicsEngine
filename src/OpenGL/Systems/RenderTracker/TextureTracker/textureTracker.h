#ifndef ENGINE_OPENGL_SYSTEM_RENDERTRACKER_TEXTURETRACKER
#define ENGINE_OPENGL_SYSTEM_RENDERTRACKER_TEXTURETRACKER

#include "./textureIndex.h"
#include <filesystem>
#include <functional>
#include <memory>

namespace Engine
{
class Registry;
class OpenGLTextureComponent;

namespace Systems
{

class OpenGLRenderTextureTracker
{
public:
    OpenGLRenderTextureTracker() = delete;
    OpenGLRenderTextureTracker(const OpenGLRenderTextureTracker &) = delete;
    OpenGLRenderTextureTracker(OpenGLRenderTextureTracker &&otherTracker) = delete;
    OpenGLRenderTextureTracker(unsigned int entity, Registry &registry, std::shared_ptr<OpenGLTextureIndex> index);

    ~OpenGLRenderTextureTracker();

    void bindTextures();
    void unbindTextures();

private:
    Registry &m_registry;
    unsigned int m_numTextures{0};

    using textureData = std::tuple<std::filesystem::path, unsigned int, unsigned int>;
    std::vector<textureData> m_textures{};

    std::shared_ptr<OpenGLTextureIndex> m_index;

    using texture_callback = std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<OpenGLTextureComponent>)>>;

    texture_callback m_updateCallback;

    void update(OpenGLTextureComponent *texture);
};

} // namespace Systems
} // namespace Engine

#endif