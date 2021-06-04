#ifndef ENGINE_OPENGL_SYSTEM_RENDERTRACKER
#define ENGINE_OPENGL_SYSTEM_RENDERTRACKER

#include "../../../Core/Components/Geometry/geometry.h"
#include "../../../Core/Components/Material/material.h"
#include "../../../Core/Components/Render/render.h"
#include "../../../Core/Components/Transform/transform.h"
#include "../../../Core/ECS/registry.h"
#include "../../Components/Shader/shader.h"
#include <map>

namespace Engine
{

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

    using geometryData = std::tuple<componentCallback<GeometryComponent>,
                                    unsigned int,
                                    unsigned int,
                                    unsigned int,
                                    unsigned int,
                                    unsigned int>; // change callback,
                                                   // VAO, VBO, EBO, num points, num faces
    std::map<GeometryComponent *, geometryData> m_geometries;

    using materialData =
        std::tuple<componentCallback<MaterialComponent>, unsigned int, int>; // change callback, UBO, size
    std::map<MaterialComponent *, materialData> m_materials;

    using transformData = std::tuple<componentCallback<TransformComponent>, unsigned int>; // change callback, UBO
    std::map<TransformComponent *, transformData> m_transforms;

    using shaderData =
        std::tuple<componentCallback<OpenGLShaderComponent>, std::vector<unsigned int>>; // change callback, entities
    std::map<OpenGLShaderComponent *, shaderData> m_shaders;

    using entityData =
        std::tuple<GeometryComponent *, MaterialComponent *, TransformComponent *, OpenGLShaderComponent *>;
    std::map<unsigned int, entityData> m_entities;

    void makeRenderable(unsigned int entityId);

    void ensureGeometry(unsigned int entity, std::shared_ptr<GeometryComponent> geometry);
    void updateGeometryData(std::shared_ptr<GeometryComponent> geometry);

    void ensureMaterial(unsigned int entity, std::shared_ptr<MaterialComponent> material);
    void updateMaterialData(std::shared_ptr<MaterialComponent> material);

    void ensureTransform(unsigned int entity, std::shared_ptr<TransformComponent> transform);
    void updateTransformData(std::shared_ptr<TransformComponent> transform);

    void ensureShader(unsigned int entity, std::shared_ptr<OpenGLShaderComponent> shader);
    void updateWithShaderData(std::shared_ptr<OpenGLShaderComponent> shader);
};

} // namespace Systems

} // namespace Engine

#endif