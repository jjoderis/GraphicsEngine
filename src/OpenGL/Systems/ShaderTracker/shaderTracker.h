#ifndef ENGINE_OPENGL_SYSTEM_SHADERTRACKER
#define ENGINE_OPENGL_SYSTEM_SHADERTRACKER

#include <functional>
#include <memory>

namespace Engine
{
class Registry;
class OpenGLShaderComponent;

namespace Systems
{

class OpenGLShaderTracker
{
public:
    OpenGLShaderTracker() = delete;
    OpenGLShaderTracker(const OpenGLShaderTracker &) = delete;
    OpenGLShaderTracker(OpenGLShaderTracker &&otherTracker) = delete;
    OpenGLShaderTracker(Registry &registry);

private:
    Registry &m_registry;

    using shader_callback =
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<Engine::OpenGLShaderComponent>)>>;

    shader_callback m_addCallback;
    shader_callback m_updateCallback;
    shader_callback m_swapCallback;

    void update(unsigned int entity, OpenGLShaderComponent *shader);
};

} // namespace Systems
} // namespace Engine

#endif