#ifndef ENGINE_OPENGL_SYSTEM_TRANSFORMTRACKER
#define ENGINE_OPENGL_SYSTEM_TRANSFORMTRACKER

#include <functional>
#include <memory>

namespace Engine
{
class Registry;
class TransformComponent;

namespace Systems
{

class OpenGLTransformTracker
{
public:
    OpenGLTransformTracker() = delete;
    OpenGLTransformTracker(const OpenGLTransformTracker &) = delete;
    OpenGLTransformTracker(OpenGLTransformTracker &&otherTracker) = delete;
    OpenGLTransformTracker(Registry &registry);

private:
    Registry &m_registry;

    using transform_callback =
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<Engine::TransformComponent>)>>;

    transform_callback m_updateCallback;

    transform_callback m_removeCallback;

    transform_callback m_swapCallback;

    void update(unsigned int entity, TransformComponent *geometry);
    void remove(unsigned int entity);
};

} // namespace Systems
} // namespace Engine

#endif