#ifndef ENGINE_OPENGL_SYSTEM_RENDERTRACKER_TRANSFORMTRACKER
#define ENGINE_OPENGL_SYSTEM_RENDERTRACKER_TRANSFORMTRACKER

#include <functional>
#include <memory>

namespace Engine
{
class Registry;
class TransformComponent;

namespace Systems
{

class OpenGLRenderTransformTracker
{
public:
    OpenGLRenderTransformTracker() = delete;
    OpenGLRenderTransformTracker(const OpenGLRenderTransformTracker &) = delete;
    OpenGLRenderTransformTracker(OpenGLRenderTransformTracker &&otherTracker) = delete;
    OpenGLRenderTransformTracker(unsigned int entity, Registry &registry);

    ~OpenGLRenderTransformTracker();

    unsigned int getBuffer();

private:
    Registry &m_registry;
    unsigned int m_UBO{0};

    using transform_callback =
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<Engine::TransformComponent>)>>;

    transform_callback m_updateCallback;

    void update(TransformComponent *transform);
};

} // namespace Systems
} // namespace Engine

#endif