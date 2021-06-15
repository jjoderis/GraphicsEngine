#ifndef ENGINE_OPENGL_SYSTEM_GEOMETRYTRACKER
#define ENGINE_OPENGL_SYSTEM_GEOMETRYTRACKER

#include <functional>
#include <memory>

namespace Engine
{
class Registry;
class GeometryComponent;

namespace Systems
{

class OpenGLGeometryTracker
{
public:
    OpenGLGeometryTracker() = delete;
    OpenGLGeometryTracker(const OpenGLGeometryTracker &) = delete;
    OpenGLGeometryTracker(OpenGLGeometryTracker &&otherTracker) = delete;
    OpenGLGeometryTracker(Registry &registry);

private:
    Registry &m_registry;

    using geometry_callback =
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<Engine::GeometryComponent>)>>;

    geometry_callback m_updateCallback;

    geometry_callback m_removeCallback;

    geometry_callback m_swapCallback;

    void update(unsigned int entity, GeometryComponent *geometry);
    void remove(unsigned int entity, GeometryComponent *geometry);
};

} // namespace Systems
} // namespace Engine

#endif