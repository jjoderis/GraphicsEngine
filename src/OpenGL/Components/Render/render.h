#ifndef OPENGL_COMPONENTS_RENDER
#define OPENGL_COMPONENTS_RENDER

#include "../../../Core/ECS/registry.h"
#include "../../../Core/Components/Geometry/geometry.h"
#include "../../../Core/Components/Material/material.h"
#include "../../../Core/Components/Transform/transform.h"
#include <glad/glad.h>
#include <map>
#include <tuple>

namespace Engine {
    class OpenGLRenderComponent {
    private:
        unsigned int m_VAO{};
        unsigned int m_VBO{}; // stores vertex data
        unsigned int m_EBO{}; // stores index data
        unsigned int m_program{}; // the shader program to use
        Engine::Registry &m_registry; // the registry holding all entities and components

        // stores the offsets of the specific Geometry into the Vertex and Index Buffer; later transform and material (index of the material used for an entity)
        using meta_data = std::tuple<
            unsigned int, // start in VBO
            unsigned int, // end in VBO
            unsigned int, // start in EBO
            unsigned int, // end in EBO
            std::shared_ptr<std::function<void(unsigned int, GeometryComponent*)>>, // callback for when a GeometryComponent is added to the entity
            std::shared_ptr<std::function<void(unsigned int, GeometryComponent*)>>, // callback for when a GeometryComponent is removed from the entity
            std::shared_ptr<std::function<void(unsigned int, GeometryComponent*)>> // callback for when a GeometryComponent is updated to the entity
        >;
        std::map<unsigned int, meta_data> m_entityData;
        unsigned int m_vboSize{0};
        unsigned int m_eboSize{0};
        unsigned int m_numPrimitives{0};

        void extendGeometryBuffers(unsigned int entityId, GeometryComponent* geometry);
        void extendMaterialBuffers(unsigned int entityId, MaterialComponent* material);
        void extendTransformBuffers(unsigned int entityId, TransformComponent* transform);

        // TODO: replace with multi material buffer and multi transform buffer implementation
        Math::Vector4 m_color;
        Math::Matrix4 m_modelMatrix;
    public:
        OpenGLRenderComponent(Engine::Registry &registry);
        ~OpenGLRenderComponent();

        // associates the render component with an entity 
        // allocates memory in buffers and stores offsets in map for Geometry associated with entity
        void associate(unsigned int entity);

        // dissassociates the render component with an entity
        void dissassociate(unsigned int entity);

        void updateVertex(unsigned int entity, int vertexIndex, Math::Vector3& newVertexData);
        void updateFace(unsigned int entity, int faceIndex, const unsigned int* newFace);
        void updateColor(unsigned int entity, Math::Vector4& newColor);
        void updateTransform(unsigned int entity, Math::Matrix4& modelMatrix);

        void render();
    };
}

#endif