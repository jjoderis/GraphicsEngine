#ifndef ENGINE_OPENGL_SYSTEM_RENDERTRACKER
#define ENGINE_OPENGL_SYSTEM_RENDERTRACKER

#include <functional>
#include <list>
#include <map>
#include <memory>

#include "./ShaderTracker/shaderTracker.h"

namespace Engine
{
class Registry;
class RenderComponent;

namespace Systems
{

class OpenGLRenderTracker
{
public:
    OpenGLRenderTracker() = delete;

    OpenGLRenderTracker(Registry &registry);

private:
    Registry &m_registry;

    // callback type for component events (add, update, remove)
    using render_callback = std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<Engine::RenderComponent>)>>;

    // callback that tracks addition of new render components
    render_callback m_addCallback;

    void makeRenderable(unsigned int entityId);

    void ensureGeometry(unsigned int entity);

    void ensureMaterial(unsigned int entity);

    void ensureTransform(unsigned int entity);

    void ensureShader(unsigned int entity);

    void ensureTexture(unsigned int entity);
};

} // namespace Systems

} // namespace Engine

#endif