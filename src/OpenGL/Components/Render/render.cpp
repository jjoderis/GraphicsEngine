#include "render.h"

struct Material {
    float color[4];
};

float modelMatrix[16]{
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};


Engine::OpenGLRenderComponent::OpenGLRenderComponent(Engine::Registry &registry) : m_registry{registry} {
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

    // setup material buffer and insert base material
    glGenBuffers(1, &m_materialUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_materialUBO);
    Material baseMat{1.0, 0.01, 0.6, 1.0};
    unsigned int blockIndex = m_program.getBlockIndex("Materials");
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), &baseMat, GL_DYNAMIC_DRAW);
    glUniformBlockBinding(m_program.getProgram(), blockIndex, 0);

    // setup transform buffer and insert base model transform
    glGenBuffers(1, &m_transformUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_transformUBO);
    blockIndex = m_program.getBlockIndex("Transforms");
    glBufferData(GL_UNIFORM_BUFFER, sizeof(modelMatrix), modelMatrix, GL_DYNAMIC_DRAW);
    glUniformBlockBinding(m_program.getProgram(), blockIndex, 1);
}

Engine::OpenGLRenderComponent::~OpenGLRenderComponent() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

size_t stride{5 * sizeof(float)};
size_t vertexSize{3 * sizeof(float)};
size_t transIndexOffset{4 * sizeof(float)};

void Engine::OpenGLRenderComponent::addToGeometryBuffers(unsigned int entityId, Engine::GeometryComponent* geometry) {
    meta_data &entityData = m_entityData.at(entityId);
    std::vector<Math::Vector3>& vertices{ geometry->getVertices() };
    size_t geometryVertexSize = 5 * sizeof(float) * vertices.size();

    // create new buffer with space for the current geometries + space for the new geometry
    unsigned int newVBO{0};
    glGenBuffers(1, &newVBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, newVBO);
    glBufferData(GL_ARRAY_BUFFER, m_vboSize + geometryVertexSize, NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, (void*)vertexSize);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, (void*)(transIndexOffset));
    glEnableVertexAttribArray(2);

    // copy the data from the old buffer into the new one
    if (m_vboSize) {
        glBindBuffer(GL_COPY_READ_BUFFER, m_VBO);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, m_vboSize);
    }

    // delete the old buffer and update the meta data
    glDeleteBuffers(1, &m_VBO);
    m_VBO = newVBO;

    std::get<0>(entityData) = m_numPoints;
    std::get<1>(entityData) = m_numPoints + vertices.size();

    m_vboSize += geometryVertexSize;
    m_numPoints += vertices.size();

    std::vector<unsigned int>& faces{geometry->getFaces()};
    size_t geometryFaceSize{sizeof(unsigned int) * faces.size()};

    // create new index buffer with space for the old and new indices;
    unsigned int newEBO{};
    glGenBuffers(1, &newEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newEBO);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_eboSize + geometryFaceSize, NULL, GL_DYNAMIC_DRAW);

    // copy the data from the old buffer into the new one
    if (m_eboSize) {
        glBindBuffer(GL_COPY_READ_BUFFER, m_EBO);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ELEMENT_ARRAY_BUFFER, 0, 0, m_eboSize);
    }

    glDeleteBuffers(1, &m_EBO);

    m_EBO = newEBO;

    std::get<2>(entityData) = m_eboSize;
    std::get<3>(entityData) = m_eboSize + geometryFaceSize;

    updateGeometryData(entityId, geometry);
    updateMaterialIndices(entityId, geometry);
    updateTransformIndices(entityId, geometry);

    size_t numVertices{vertices.size()};
    size_t numFaces{faces.size()};
    // subscribe for updates on the geometry to update the buffer data accordingly
    std::get<6>(entityData) = m_registry.onUpdate<GeometryComponent>(entityId, [=](unsigned int entity, GeometryComponent* geometry) {
        if (numVertices == geometry->getVertices().size() || numFaces == geometry->getFaces().size()) {
            this->updateGeometryData(entityId, geometry);
        }
        // TODO: remove and readd if number of vertices or faces changes
    });

    m_eboSize += geometryFaceSize;
    m_numFaces += faces.size();
    calculatePrimtiveCount();
}

void Engine::OpenGLRenderComponent::calculatePrimtiveCount() {
    if (m_primitiveType == GL_POINTS) {
        m_numPrimitives = m_numPoints;
    } else {
        m_numPrimitives = m_numFaces;
    }
}

void Engine::OpenGLRenderComponent::updateGeometryData(unsigned int entityId, GeometryComponent* geometry) {
    std::vector<Math::Vector3>& vertices{ geometry->getVertices() };
    size_t vertexOffset{std::get<0>(m_entityData.at(entityId))};
    size_t objectStart{vertexOffset * stride};

    for (int i = 0; i < vertices.size(); ++i) 
    {  
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, objectStart + stride * i, vertexSize, vertices[i].raw());
    }

    // we need to make a copy because we have to offset all face indices by the amount of vertices comping before this object
    std::vector<unsigned int> faces{geometry->getFaces()};
    for(unsigned int& face: faces) {
        face += vertexOffset;
    }
    size_t objectFaceStart{std::get<2>(m_entityData.at(entityId))};
    // copy the data of the new geometry into the new buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, objectFaceStart, faces.size() * sizeof(unsigned int), faces.data());
}

void Engine::OpenGLRenderComponent::updateMaterialIndices(unsigned int entityId, GeometryComponent* geometry) {
    std::vector<Math::Vector3>& vertices{ geometry->getVertices() };
    float matIndex{std::get<4>(m_entityData.at(entityId))};
    size_t objectStart{std::get<0>(m_entityData.at(entityId)) * stride};

    for (int i = 0; i < vertices.size(); ++i) 
    {  
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, objectStart + stride * i + vertexSize, sizeof(float), &matIndex);
    }    
}

void Engine::OpenGLRenderComponent::updateTransformIndices(unsigned int entityId, GeometryComponent* geometry) {
    std::vector<Math::Vector3>& vertices{ geometry->getVertices() };
    float transIndex{std::get<5>(m_entityData.at(entityId))};
    size_t objectStart{std::get<0>(m_entityData.at(entityId)) * stride};

    for (int i = 0; i < vertices.size(); ++i) 
    {  
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, objectStart + stride * i + transIndexOffset, sizeof(float), &transIndex);
    }
}

void Engine::OpenGLRenderComponent::updatePrimitiveType(int primitiveType) {
    m_primitiveType = primitiveType;
    calculatePrimtiveCount();
}

void Engine::OpenGLRenderComponent::updateMaterialBuffers(unsigned int entityId, MaterialComponent* material) {
    const std::list<unsigned int>& owners = m_registry.getOwners<MaterialComponent>(material);
    float& matIndex = std::get<4>(m_entityData.at(entityId));

    for (unsigned int owner: owners) {
        if (m_entityData.find(owner) != m_entityData.end()) {
            float ownerMatIndex = std::get<4>(m_entityData.at(owner));
            if (ownerMatIndex) {
                // material already known; just copy index
                matIndex = ownerMatIndex;

                // handle removal of the material from the given entity (use standard material)
                std::get<9>(m_entityData.at(entityId)) = m_registry.onRemove<MaterialComponent>([=](unsigned int entity, MaterialComponent* material) {
                    if (entity == entityId) {
                        std::get<4>(this->m_entityData.at(entity)) = 0;
                        this->updateMaterialIndices(entity, m_registry.getComponent<Engine::GeometryComponent>(entity));
                    }
                });
                return;
            }
        }
    }

    // material not already known
    matIndex = m_numMaterials;

    unsigned int newMaterialUBO{};
    glGenBuffers(1, &newMaterialUBO);
    glBindBuffer(GL_COPY_READ_BUFFER, m_materialUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, newMaterialUBO);
    glBufferData(GL_UNIFORM_BUFFER, (m_numMaterials + 1) * sizeof(Material), NULL, GL_DYNAMIC_DRAW);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_UNIFORM_BUFFER, 0, 0, m_numMaterials * sizeof(Material));
    
    glUniformBlockBinding(m_program.getProgram(), m_program.getBlockIndex("Materials"), 0);

    glDeleteBuffers(1, &m_materialUBO);
    m_materialUBO = newMaterialUBO;

    updateMaterialData(entityId, material);

    // subscribe to material updates
    std::get<8>(m_entityData.at(entityId)) = m_registry.onUpdate<MaterialComponent>(entityId, [this, entityId] (unsigned int entity, MaterialComponent* material) {
        this->updateMaterialData(entityId, material);
    });

    // TODO: handle removal from "owner entity" check if some other entity still references it and make it the owner else remmove from buffer and update all indices accordingly

    ++m_numMaterials;
}

void Engine::OpenGLRenderComponent::updateMaterialData(unsigned int entity, MaterialComponent* material) {
    glBindBuffer(GL_UNIFORM_BUFFER, this->m_materialUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, std::get<4>(this->m_entityData.at(entity)) * sizeof(Material), sizeof(Material), material->getColor().raw());
}

void Engine::OpenGLRenderComponent::updateTransformBuffers(unsigned int entityId, TransformComponent* transform) {
    const std::list<unsigned int>& owners = m_registry.getOwners<TransformComponent>(transform);
    float& transIndex = std::get<5>(m_entityData.at(entityId));

    for (unsigned int owner: owners) {
        if (m_entityData.find(owner) != m_entityData.end()) {
            float ownerTransIndex = std::get<5>(m_entityData.at(owner));
            if (ownerTransIndex) {
                // transform already known; just copy index
                transIndex = ownerTransIndex;

                // handle removal of the transform from the given entity (use standard transform)
                std::get<11>(m_entityData.at(entityId)) = m_registry.onRemove<TransformComponent>([=](unsigned int entity, TransformComponent* transform) {
                    if (entity == entityId) {
                        std::get<5>(this->m_entityData.at(entity)) = 0;
                        this->updateTransformIndices(entity, m_registry.getComponent<Engine::GeometryComponent>(entity));
                    }
                });
                return;
            }
        }
    }

    // transform not already known
    transIndex = m_numTransforms;

    unsigned int newTransformUBO{};
    glGenBuffers(1, &newTransformUBO);
    glBindBuffer(GL_COPY_READ_BUFFER, m_transformUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, newTransformUBO);
    glBufferData(GL_UNIFORM_BUFFER, (m_numTransforms + 1) * sizeof(modelMatrix), NULL, GL_DYNAMIC_DRAW);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_UNIFORM_BUFFER, 0, 0, m_numTransforms * sizeof(modelMatrix));
    
    glUniformBlockBinding(m_program.getProgram(), m_program.getBlockIndex("Transforms"), 1);

    glDeleteBuffers(1, &m_transformUBO);
    m_transformUBO = newTransformUBO;

    updateTransformData(entityId, transform);

    // subscribe to transform updates
    std::get<10>(m_entityData.at(entityId)) = m_registry.onUpdate<TransformComponent>(entityId, [this, entityId] (unsigned int entity, TransformComponent* transform) {
        this->updateTransformData(entityId, transform);
    });

    ++m_numTransforms;
}

void Engine::OpenGLRenderComponent::updateTransformData(unsigned int entity, TransformComponent* transform) {
    glBindBuffer(GL_UNIFORM_BUFFER, this->m_transformUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, std::get<5>(this->m_entityData.at(entity)) * sizeof(modelMatrix), sizeof(modelMatrix), transform->getModelMatrix().raw());
}

void Engine::OpenGLRenderComponent::associate(unsigned int entity) {
    // if the entity is already associated with this component do nothing
    if (m_entityData.find(entity) != m_entityData.end()) {
        return;
    }

    m_entityData.emplace(entity, std::tuple<
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
            std::shared_ptr<std::function<void(unsigned int, TransformComponent*)>>  // callback for when a MaterialComponent is removed from the entity
        >{
        -1, -1, -1, -1, 0.0, 0.0,
        std::shared_ptr<std::function<void(unsigned int, GeometryComponent*)>>{},
        std::shared_ptr<std::function<void(unsigned int, GeometryComponent*)>>{},
        std::shared_ptr<std::function<void(unsigned int, MaterialComponent*)>>{},
        std::shared_ptr<std::function<void(unsigned int, MaterialComponent*)>>{},
        std::shared_ptr<std::function<void(unsigned int, TransformComponent*)>>{},
        std::shared_ptr<std::function<void(unsigned int, TransformComponent*)>>{} 
    });

    MaterialComponent* material = m_registry.getComponent<Engine::MaterialComponent>(entity);
    if (material) {
        updateMaterialBuffers(entity, material);
    }

    TransformComponent* transform = m_registry.getComponent<TransformComponent>(entity);
    if (transform) {
        updateTransformBuffers(entity, transform);
    }

    // entity has a geometry component => setup buffers to use geometry data of that component
    GeometryComponent* geometry = m_registry.getComponent<Engine::GeometryComponent>(entity);
    if (geometry && material) {
        addToGeometryBuffers(entity, geometry);
        // TODO: set callback on component to update the buffers when the geometry is updated
    } else {
        // TODO: setup callback that calls the insert function as soon as a geometry becomes available
    }

    // TODO: setup of material
    // TODO: set callback on entity to dissassociate on removal of components
}

void Engine::OpenGLRenderComponent::dissassociate(unsigned int entity) {
    // TODO: remove components from buffers
}

void Engine::OpenGLRenderComponent::render() {
    m_program.use();

    glBindVertexArray(m_VAO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_materialUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_transformUBO);

    if (m_numPrimitives == GL_POINTS) {
        glDrawArrays(GL_POINTS, 0, m_numPrimitives);
    } else {
        glDrawElements(m_primitiveType, m_numPrimitives, GL_UNSIGNED_INT, 0);
    }
}