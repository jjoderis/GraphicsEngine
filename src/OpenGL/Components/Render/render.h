#ifndef ENGINE_OPENGL_COMPONENTS_RENDER
#define ENGINE_OPENGL_COMPONENTS_RENDER

#include "../../../Core/ECS/registry.h"
#include "../../../Core/Components/Geometry/geometry.h"
#include "../../../Core/Components/Material/material.h"
#include "../../../Core/Components/Transform/transform.h"
#include "../../Shader/shader.h"
#include <glad/glad.h>
#include <map>
#include <tuple>

class RenderTest;
namespace Engine {
    class OpenGLRenderComponent {
    private:
        // waits for the component to be added to an entity to then set up callbacks to react to changes in the components of that entity
        std::shared_ptr<std::function<void(unsigned int, OpenGLRenderComponent*)>> m_associateCallback;

        unsigned int m_VAO{};
        unsigned int m_VBO{}; // stores vertex data
        unsigned int m_EBO{}; // stores index data
        unsigned int m_materialUBO{}; // stores material data
        unsigned int m_transformUBO{}; // stores transform data
        Engine::OpenGLProgram m_program{}; // the shader program to use
        Engine::Registry &m_registry; // the registry holding all entities and components

        // stores the offsets of the specific Geometry into the Vertex and Index Buffer; later transform and material (index of the material used for an entity)
        using meta_data = std::tuple<
            unsigned int, // start vert count
            unsigned int, // end vert count
            unsigned int, // start in EBO
            unsigned int, // end in EBO
            float, // material index
            float, // transform index
            std::shared_ptr<std::function<void(unsigned int, GeometryComponent*)>>, // callback for when a GeometryComponent is added or updated to the entity
            std::shared_ptr<std::function<void(unsigned int, GeometryComponent*)>>, // callback for when a GeometryComponent is removed from the entity
            std::shared_ptr<std::function<void(unsigned int, MaterialComponent*)>>, // callback for when a MaterialComponent is added or updated to the entity
            std::shared_ptr<std::function<void(unsigned int, MaterialComponent*)>>, // callback for when a MaterialComponent is removed from the entity
            std::shared_ptr<std::function<void(unsigned int, TransformComponent*)>>, // callback for when a MaterialComponent is added or updated to the entity
            std::shared_ptr<std::function<void(unsigned int, TransformComponent*)>>,  // callback for when a MaterialComponent is removed from the entity
            std::shared_ptr<std::function<void(unsigned int, OpenGLRenderComponent*)>> // callback for when the Render component is removed from the entity
        >;
        std::map<unsigned int, meta_data> m_entityData;
        unsigned int m_numPoints{0};
        unsigned int m_numFaces{0};
        unsigned int m_numPrimitives{0};
        unsigned int m_primitiveType{GL_TRIANGLES};

        unsigned int m_numMaterials{1};
        unsigned int m_numTransforms{1};

        void setupEntity(unsigned int entity, GeometryComponent* geometry);
        void teardownEntity(unsigned int entity);

        void addVertices(unsigned int entity, GeometryComponent* geometry);
        void updateVertices(unsigned int entity, GeometryComponent* geometry);
        void removeVertices(unsigned int entity);

        void addFaces(unsigned int entity, GeometryComponent* geometry);
        void updateFaces(unsigned int entity, GeometryComponent* geometry);
        void removeFaces(unsigned int entity);

        void addMaterial(unsigned int entity, MaterialComponent* material);
        void updateMaterial(unsigned int entity, MaterialComponent* material);
        void updateMaterialIndices(unsigned int entity);
        void removeMaterial(unsigned int entity, MaterialComponent* material);
        

        void addTransform(unsigned int entityId, TransformComponent* transform);
        void updateTransform(unsigned int entity, TransformComponent* transform);
        void updateTransformIndices(unsigned int entitiyId);
        void removeTransform(unsigned int entity, TransformComponent* transform);

        void calculatePrimitiveCount();

        friend RenderTest;
    public:
        OpenGLRenderComponent(Engine::Registry &registry);
        ~OpenGLRenderComponent();

        // associates the render component with an entity 
        // allocates memory in buffers and stores offsets in map for Geometry associated with entity
        void associate(unsigned int entity);

        // dissassociates the render component with an entity
        void dissassociate(unsigned int entity);

        void updatePrimitiveType(int primitiveType);
        void updateTransform(unsigned int entity, Math::Matrix4& modelMatrix);

        void render();
    };
}

#endif