#ifndef ENGINE_OPENGL_SYSTEM_RENDERTRACKER
#define ENGINE_OPENGL_SYSTEM_RENDERTRACKER

#include <functional>
#include <list>
#include <map>
#include <memory>

#include "./GeometryTracker/geometryTracker.h"
#include "./MaterialTracker/materialTracker.h"
#include "./ShaderTracker/shaderTracker.h"
#include "./TransformTracker/transformTracker.h"

namespace Engine
{
class Registry;
class RenderComponent;
class GeometryComponent;
class OpenGLMaterialComponent;
class OpenGLShaderComponent;
class TransformComponent;

namespace Systems
{

class OpenGLRenderTracker
{
public:
    OpenGLRenderTracker() = delete;

    OpenGLRenderTracker(Registry &registry);

    void render();

private:
    Registry &m_registry;

    // callback type for component events (add, update, remove)
    template <typename T>
    using componentCallback = std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<T>)>>;

    // callback that tracks addition of new render components
    componentCallback<RenderComponent> m_associateCallback;

    std::map<GeometryComponent *, OpenGLRenderGeometryTracker> m_geometries;

    std::map<OpenGLMaterialComponent *, OpenGLRenderMaterialTracker> m_materials;

    std::map<TransformComponent *, OpenGLRenderTransformTracker> m_transforms;

    std::map<OpenGLShaderComponent *, OpenGLRenderShaderTracker> m_shaders;

    using entityData =
        std::tuple<GeometryComponent *, OpenGLMaterialComponent *, TransformComponent *, OpenGLShaderComponent *>;
    std::map<unsigned int, entityData> m_entities;

    void makeRenderable(unsigned int entityId);

    void ensureGeometry(unsigned int entity);

    void ensureMaterial(unsigned int entity);

    void ensureTransform(unsigned int entity);

    void ensureShader(unsigned int entity);
};

} // namespace Systems

} // namespace Engine

#endif