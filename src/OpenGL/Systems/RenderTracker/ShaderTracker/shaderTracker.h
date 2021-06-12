#ifndef ENGINE_OPENGL_SYSTEM_RENDERTRACKER_SHADERTRACKER
#define ENGINE_OPENGL_SYSTEM_RENDERTRACKER_SHADERTRACKER

#include <functional>
#include <list>
#include <memory>

namespace Engine
{
class Registry;
class OpenGLShaderComponent;

namespace Systems
{

class OpenGLRenderShaderTracker
{
public:
    OpenGLRenderShaderTracker() = delete;
    OpenGLRenderShaderTracker(const OpenGLRenderShaderTracker &) = delete;
    OpenGLRenderShaderTracker(OpenGLRenderShaderTracker &&otherTracker) = delete;
    OpenGLRenderShaderTracker(unsigned int entity, Registry &registry);

    const std::list<unsigned int> &getUsers() const;

private:
    Registry &m_registry;

    std::list<unsigned int> m_users;

    using shader_callback =
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<Engine::OpenGLShaderComponent>)>>;

    shader_callback m_updateCallback;

    void update(OpenGLShaderComponent *shader);
};

} // namespace Systems
} // namespace Engine

#endif