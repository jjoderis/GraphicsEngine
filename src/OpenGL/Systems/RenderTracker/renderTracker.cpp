#include "renderTracker.h"

#include "../../../Core/Components/Geometry/geometry.h"
#include "../../../Core/Components/Render/render.h"
#include "../../../Core/Components/Transform/transform.h"
#include "../../../Core/ECS/registry.h"
#include "../../Components/Material/material.h"
#include "../../Components/Shader/shader.h"

Engine::Systems::OpenGLRenderTracker::OpenGLRenderTracker(Registry &registry) : m_registry{registry}
{
    m_associateCallback = m_registry.onAdded<RenderComponent>(
        [this](unsigned int entity, std::weak_ptr<RenderComponent> render)
        {
            if (m_entities.find(entity) == m_entities.end())
            {
                this->makeRenderable(entity);
            }
        });
    // TODO: handle removal
}

void Engine::Systems::OpenGLRenderTracker::makeRenderable(unsigned int entity)
{
    m_entities.emplace(entity, entityData{nullptr, nullptr, nullptr, nullptr});

    // make sure we have every necessary component to render
    std::shared_ptr<OpenGLMaterialComponent> material{m_registry.getComponent<OpenGLMaterialComponent>(entity)};
    if (!material)
    {
        material =
            m_registry.addComponent<OpenGLMaterialComponent>(entity, std::make_shared<OpenGLMaterialComponent>());
    }
    ensureMaterial(entity, material);

    std::shared_ptr<GeometryComponent> geometry{m_registry.getComponent<GeometryComponent>(entity)};
    if (!geometry)
    {
        geometry = m_registry.addComponent<GeometryComponent>(
            entity,
            std::make_shared<Engine::GeometryComponent>(
                std::initializer_list<Engine::Math::Vector3>{Engine::Math::Vector3{0.5, -0.5, 0.0},
                                                             Engine::Math::Vector3{-0.5, -0.5, 0.0},
                                                             Engine::Math::Vector3{0.0, 0.5, 0.0}},
                std::initializer_list<unsigned int>{0, 1, 2}));
        geometry->calculateNormals();
    }
    ensureGeometry(entity, geometry);

    std::shared_ptr<TransformComponent> transform{m_registry.getComponent<TransformComponent>(entity)};
    if (!transform)
    {
        transform = m_registry.addComponent<TransformComponent>(entity, std::make_shared<TransformComponent>());
    }
    ensureTransform(entity, transform);

    // we expect the shader to exist for now since creating the shader here would need some kind of base
    // shader code to be defined here
    std::shared_ptr<OpenGLShaderComponent> shader{m_registry.getComponent<OpenGLShaderComponent>(entity)};
    ensureShader(entity, shader);
    /**
     * TODO: setup callbacks for changes on all needed components ONCE
     * Components should not be removable while the Render Component is on them
     * what happens if a component gets swapped out on one entity?
     **/
}

void Engine::Systems::OpenGLRenderTracker::ensureGeometry(unsigned int entity,
                                                          std::shared_ptr<GeometryComponent> geometry)
{
    // check if the geometry is not currently known
    if (m_geometries.find(geometry.get()) == m_geometries.end())
    {
        m_geometries.emplace(geometry.get(),
                             geometryData{m_registry.onUpdate<GeometryComponent>(
                                              entity,
                                              [&](unsigned int updateEntity, std::weak_ptr<GeometryComponent> weakGeo)
                                              {
                                                  // check if same geometry?
                                                  updateGeometryData(weakGeo.lock());
                                              }),
                                          0,
                                          0,
                                          0,
                                          0,
                                          0});

        updateGeometryData(geometry);
    }

    // TODO: check if the entity currently has another geometry and if yes check if that geometry is in use by at least
    // another entity,
    // if not -> delete

    std::get<0>(m_entities.at(entity)) = geometry.get();
}

constexpr size_t positionSize{3 * sizeof(float)};
constexpr size_t normalSize{3 * sizeof(float)};
constexpr size_t pointSize{positionSize + normalSize};
constexpr size_t faceSize{sizeof(unsigned int)};

void Engine::Systems::OpenGLRenderTracker::updateGeometryData(std::shared_ptr<GeometryComponent> geometry)
{
    geometryData &geoData{m_geometries.at(geometry.get())};
    unsigned int &VAO{std::get<1>(geoData)};
    unsigned int &VBO{std::get<2>(geoData)};
    unsigned int &EBO{std::get<3>(geoData)};
    unsigned int &numPoints{std::get<4>(geoData)};
    unsigned int &numFaces{std::get<5>(geoData)};
    std::vector<Math::Vector3> &vertices{geometry->getVertices()};
    std::vector<Math::Vector3> &normals{geometry->getNormals()};
    std::vector<unsigned int> &faces{geometry->getFaces()};

    if (VAO == 0)
    {
        glGenVertexArrays(1, &VAO);
    }
    glBindVertexArray(VAO);

    // logic for when number of points or faces changes
    if (VBO > 0 && numPoints != vertices.size())
    {
        glDeleteBuffers(1, &VBO);
    }
    if (EBO > 0 && numPoints != vertices.size())
    {
        glDeleteBuffers(1, &EBO);
    }

    numPoints = vertices.size();
    numFaces = faces.size();
    if (VBO == 0)
    {
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // allocate memory for the geometry buffer
        glBufferData(GL_ARRAY_BUFFER, numPoints * pointSize, NULL, GL_DYNAMIC_DRAW);
    }
    if (EBO == 0)
    {
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        // allocate memory for the face buffer
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, numFaces * faceSize, NULL, GL_DYNAMIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    // activate positional data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, pointSize, (void *)0);
    glEnableVertexAttribArray(0);
    // activate normal data // TODO: maybe dependant on shader?
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, pointSize, (void *)positionSize);
    glEnableVertexAttribArray(1);

    for (int i = 0; i < numPoints; ++i)
    {
        glBufferSubData(GL_ARRAY_BUFFER, pointSize * i, positionSize, vertices[i].raw());
        glBufferSubData(GL_ARRAY_BUFFER, pointSize * i + positionSize, normalSize, normals[i].raw());
    }

    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, faces.size() * faceSize, faces.data());
}

void Engine::Systems::OpenGLRenderTracker::ensureMaterial(unsigned int entity,
                                                          std::shared_ptr<OpenGLMaterialComponent> material)
{
    // check if the material is not currently known
    if (m_materials.find(material.get()) == m_materials.end())
    {
        m_materials.emplace(
            material.get(),
            materialData{m_registry.onUpdate<OpenGLMaterialComponent>(
                             entity,
                             [&](unsigned int updateEntity, std::weak_ptr<OpenGLMaterialComponent> weakMat)
                             {
                                 // check if same material?
                                 updateMaterialData(weakMat.lock());
                             }),
                         0,
                         0});

        updateMaterialData(material);
    }

    // TODO: check if the entity currently has another material and if yes check if that material is in use by at least
    // another entity,
    // if not -> delete

    std::get<1>(m_entities.at(entity)) = material.get();
}

void Engine::Systems::OpenGLRenderTracker::updateMaterialData(std::shared_ptr<OpenGLMaterialComponent> material)
{
    materialData &matData{m_materials.at(material.get())};
    unsigned int &UBO{std::get<1>(matData)};
    int &size{std::get<2>(matData)};
    int materialDataSize{std::get<0>(material->getMaterialData())};

    if (UBO > 0 && size != materialDataSize)
    {
        glDeleteBuffers(1, &UBO);
        UBO = 0;
        size = materialDataSize;
    }
    if (UBO == 0)
    {
        glGenBuffers(1, &UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferData(GL_UNIFORM_BUFFER, materialDataSize, NULL, GL_DYNAMIC_DRAW);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, materialDataSize, material->getData().data());
}

void Engine::Systems::OpenGLRenderTracker::ensureTransform(unsigned int entity,
                                                           std::shared_ptr<TransformComponent> transform)
{
    // check if the transform is not currently known
    if (m_transforms.find(transform.get()) == m_transforms.end())
    {
        m_transforms.emplace(transform.get(),
                             transformData{
                                 m_registry.onUpdate<TransformComponent>(
                                     entity,
                                     [&](unsigned int updateEntity, std::weak_ptr<TransformComponent> weakTrans)
                                     {
                                         // check if same transform?
                                         updateTransformData(weakTrans.lock());
                                     }),
                                 0,
                             });

        updateTransformData(transform);
    }

    // TODO: check if the entity currently has another transform and if yes check if that transform is in use by at
    // least another entity, if not -> delete

    std::get<2>(m_entities.at(entity)) = transform.get();
}

constexpr size_t matrixSize{16 * sizeof(float)};
constexpr size_t transformSize{2 * matrixSize};

void Engine::Systems::OpenGLRenderTracker::updateTransformData(std::shared_ptr<TransformComponent> transform)
{
    transformData &transData{m_transforms.at(transform.get())};
    unsigned int &UBO{std::get<1>(transData)};

    if (UBO == 0)
    {
        glGenBuffers(1, &UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferData(GL_UNIFORM_BUFFER, transformSize, NULL, GL_DYNAMIC_DRAW);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, matrixSize, transform->getModelMatrix().raw());
    glBufferSubData(GL_UNIFORM_BUFFER, matrixSize, matrixSize, transform->getNormalMatrix().raw());
}

void Engine::Systems::OpenGLRenderTracker::ensureShader(unsigned int entity,
                                                        std::shared_ptr<OpenGLShaderComponent> shader)
{
    std::get<3>(m_entities.at(entity)) = shader.get();
    // check if the shader is not currently known
    if (m_shaders.find(shader.get()) == m_shaders.end())
    {
        m_shaders.emplace(shader.get(),
                          shaderData{
                              m_registry.onUpdate<OpenGLShaderComponent>(
                                  entity,
                                  [&](unsigned int updateEntity, std::weak_ptr<OpenGLShaderComponent> weakShader)
                                  {
                                      // check if same shader?
                                      updateWithShaderData(weakShader.lock());
                                  }),
                              std::vector<unsigned int>{},
                          });
    }
    // TODO: check if the entity currently has another shader and if yes check if that shader is in use by at
    // least another entity, if not -> delete
    std::get<1>(m_shaders.at(shader.get())).emplace_back(entity);

    // update current entities material with shader data
    updateWithShaderData(shader);
}

// supposed to update the related materials with new uniform data
void Engine::Systems::OpenGLRenderTracker::updateWithShaderData(std::shared_ptr<OpenGLShaderComponent> shader)
{
    ShaderMaterialData matData{shader->getMaterialProperties()};

    for (unsigned int entity : std::get<1>(m_shaders.at(shader.get())))
    {
        // TODO: prevent changing the same material multiple times
        std::get<1>(m_entities.at(entity))->setMaterialData(matData);
    }
}

void Engine::Systems::OpenGLRenderTracker::render()
{
    for (auto const &shaderMapEntry : m_shaders)
    {
        shaderMapEntry.first->useShader();

        const std::vector<unsigned int> &entities{std::get<1>(shaderMapEntry.second)};

        for (unsigned int entity : entities)
        {
            entityData &data{m_entities.at(entity)};
            geometryData &geoData{m_geometries.at(std::get<0>(data))};
            unsigned int VAO{std::get<1>(geoData)};
            unsigned int numFaces{std::get<5>(geoData)};
            unsigned int materialUBO{std::get<1>(m_materials.at(std::get<1>(data)))};
            unsigned int transformUBO{std::get<1>(m_transforms.at(std::get<2>(data)))};

            glBindVertexArray(VAO);
            glBindBufferBase(GL_UNIFORM_BUFFER, 0, materialUBO);
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, transformUBO);
            glDrawElements(GL_TRIANGLES, numFaces, GL_UNSIGNED_INT, 0);
        }
    }
}
