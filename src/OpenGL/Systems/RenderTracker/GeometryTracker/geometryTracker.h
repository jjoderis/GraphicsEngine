#ifndef ENGINE_OPENGL_SYSTEM_RENDERTRACKER_GEOMETRYTRACKER
#define ENGINE_OPENGL_SYSTEM_RENDERTRACKER_GEOMETRYTRACKER

#include <functional>
#include <memory>

namespace Engine
{
class Registry;
class GeometryComponent;

namespace Systems
{

class OpenGLRenderGeometryTracker
{
public:
    OpenGLRenderGeometryTracker() = delete;
    OpenGLRenderGeometryTracker(const OpenGLRenderGeometryTracker &) = delete;
    OpenGLRenderGeometryTracker(OpenGLRenderGeometryTracker &&otherTracker) = delete;
    OpenGLRenderGeometryTracker(unsigned int entity, Registry &registry);

    ~OpenGLRenderGeometryTracker();

    void draw();

private:
    Registry &m_registry;
    int m_numPoints{0};
    int m_numFaces{0};
    bool m_useNormals{0};
    bool m_useTexCoords{0};
    unsigned int m_VAO{0};
    unsigned int m_VBO{0};
    unsigned int m_EBO{0};

    using geometry_callback =
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<Engine::GeometryComponent>)>>;

    geometry_callback m_updateCallback;

    void update(GeometryComponent *material);
};

} // namespace Systems
} // namespace Engine

#endif