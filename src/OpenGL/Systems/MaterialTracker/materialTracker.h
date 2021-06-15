#ifndef ENGINE_OPENGL_SYSTEM_MATERIALTRACKER
#define ENGINE_OPENGL_SYSTEM_MATERIALTRACKER

#include <functional>
#include <memory>

namespace Engine
{
class Registry;
class OpenGLMaterialComponent;

namespace Systems
{

class OpenGLMaterialTracker
{
public:
    OpenGLMaterialTracker() = delete;
    OpenGLMaterialTracker(const OpenGLMaterialTracker &) = delete;
    OpenGLMaterialTracker(OpenGLMaterialTracker &&otherTracker) = delete;
    OpenGLMaterialTracker(Registry &registry);

private:
    Registry &m_registry;

    using geometry_callback =
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<Engine::OpenGLMaterialComponent>)>>;

    geometry_callback m_addCallback;

    geometry_callback m_swapCallback;

    void update(unsigned int entity, OpenGLMaterialComponent *geometry);
};

} // namespace Systems
} // namespace Engine

#endif