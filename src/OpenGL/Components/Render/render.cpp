#include "render.h"

struct Material {
    float color[4];
};

float baseTransform[32]{
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0,

    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};


Engine::OpenGLRenderComponent::OpenGLRenderComponent(Engine::Registry &registry, std::initializer_list<OpenGLShader> shaders)
    : m_registry{registry}, m_program{shaders} 
{
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
    
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Material), &baseMat, GL_DYNAMIC_DRAW);
    
    // setup transform buffer and insert base model transform
    glGenBuffers(1, &m_transformUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_transformUBO);
    
    glBufferData(GL_UNIFORM_BUFFER, sizeof(baseTransform), baseTransform, GL_DYNAMIC_DRAW);

    setupUniforms();

    // add callback that is invoked every time a RenderComponent is added to an entity which then associates them
    m_associateCallback = registry.onAdded<Engine::OpenGLRenderComponent>([=](unsigned int entity, std::weak_ptr<OpenGLRenderComponent> renderComponent) {
        if (renderComponent.lock().get() == this) {
            this->associate(entity);
        }
    });
}

Engine::OpenGLRenderComponent::~OpenGLRenderComponent() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    glDeleteBuffers(1, &m_materialUBO);
    glDeleteBuffers(1, &m_transformUBO);
}

void Engine::OpenGLRenderComponent::updateShaders(std::vector<OpenGLShader>& newShaders) {
    try {
        m_program.updateProgram(newShaders);
        setupUniforms();
    } catch(ShaderException& err) {
        setupUniforms();
        throw err;
    }
}

void Engine::OpenGLRenderComponent::setupUniforms() {
    int blockIndex = m_program.getBlockIndex("Materials");
    if (blockIndex >= 0) {
        glUniformBlockBinding(m_program.getProgram(), blockIndex, 0);
    }

    blockIndex = m_program.getBlockIndex("Transforms");
    if (blockIndex >= 0) {
        glUniformBlockBinding(m_program.getProgram(), blockIndex, 1);
    }

    blockIndex = m_program.getBlockIndex("Camera");
    if (blockIndex >= 0) {
        glUniformBlockBinding(m_program.getProgram(), blockIndex, 2);
    }

    blockIndex = m_program.getBlockIndex("Lights");
    if (blockIndex >= 0) {
        glUniformBlockBinding(m_program.getProgram(), blockIndex, 3);
    }
}

size_t stride{8 * sizeof(float)};
size_t vertexSize{3 * sizeof(float)};
size_t normalSize{3* sizeof(float)};
size_t matIndexOffset{vertexSize + normalSize};
size_t transIndexOffset{matIndexOffset + sizeof(float)};

// TODO: we always expect there to be vertex normals at the moment => maybe make this dependent on the shader in the future (are normals needed)
void Engine::OpenGLRenderComponent::addVertices(unsigned int entity, const std::shared_ptr<GeometryComponent>& geometry) {
    meta_data &entityData = m_entityData.at(entity);
    std::vector<Math::Vector3>& vertices{ geometry->getVertices() };
    size_t geometryVertexSize = stride * vertices.size();
    size_t bufferSize{m_numPoints * stride};

    // create new buffer with space for the current geometries + space for the new geometry
    unsigned int newVBO{0};
    glGenBuffers(1, &newVBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, newVBO);
    glBufferData(GL_ARRAY_BUFFER, bufferSize + geometryVertexSize, NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)vertexSize);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, (void*)matIndexOffset);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride, (void*)transIndexOffset);
    glEnableVertexAttribArray(3);

    // copy the data from the old buffer into the new one
    if (bufferSize) {
        glBindBuffer(GL_COPY_READ_BUFFER, m_VBO);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, bufferSize);
    }

    // delete the old buffer and update the meta data
    glDeleteBuffers(1, &m_VBO);
    m_VBO = newVBO;

    std::get<0>(entityData) = m_numPoints;
    std::get<1>(entityData) = m_numPoints + vertices.size();

    m_numPoints += vertices.size();
}

void Engine::OpenGLRenderComponent::updateVertices(unsigned int entity, const std::shared_ptr<GeometryComponent>& geometry) {
    std::vector<Math::Vector3>& vertices{ geometry->getVertices() };
    std::vector<Math::Vector3>& normals{ geometry->getNormals() };
    size_t vertexOffset{std::get<0>(m_entityData.at(entity))};
    size_t objectStart{vertexOffset * stride};

    for (int i = 0; i < vertices.size(); ++i) 
    {  
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, objectStart + stride * i, vertexSize, vertices[i].raw());
        glBufferSubData(GL_ARRAY_BUFFER, objectStart + stride * i + vertexSize, normalSize, normals[i].raw());
    }
}

void Engine::OpenGLRenderComponent::removeVertices(unsigned int entity) {
    unsigned int& objectStart{std::get<0>(m_entityData.at(entity))}; // vertex index at which the object starts
    unsigned int& objectEnd{std::get<1>(m_entityData.at(entity))}; // vertex index ath which the object starts
    unsigned int numObjectPoints{objectEnd - objectStart};

    size_t objectSize{stride * numObjectPoints};
    size_t oldBufferSize{m_numPoints * stride};
    size_t newBufferSize{oldBufferSize - objectSize};

    glBindVertexArray(m_VAO);
    // create smaller vertex buffer and copy info of all other objects into it
    unsigned int newVBO{};
    glGenBuffers(1, &newVBO);
    glBindBuffer(GL_COPY_READ_BUFFER, m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, newVBO);
    glBufferData(GL_ARRAY_BUFFER, newBufferSize, NULL, GL_DYNAMIC_DRAW);

    size_t firstBlockSize{stride * objectStart}; // the size of the old buffer before the object to remove
    size_t secondBlockSize{stride * (m_numPoints - objectEnd)}; // the size of the old buffer behind the object to remove

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0, 0, firstBlockSize);
    // skip over the object to delete in old buffer
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, firstBlockSize + objectSize, firstBlockSize, secondBlockSize);

    glDeleteBuffers(1, &m_VBO);
    m_VBO = newVBO;

    for (auto& entry: m_entityData) {
        unsigned int& otherObjectStart{std::get<0>(entry.second)};
        if (entry.first != entity && otherObjectStart > objectStart) {
            otherObjectStart -= numObjectPoints;
            updateFaces(entry.first, m_registry.getComponent<GeometryComponent>(entry.first));
        }
    }

    objectStart = 0;
    objectEnd = 0;

    m_numPoints -= numObjectPoints;
}

void Engine::OpenGLRenderComponent::addFaces(unsigned int entity, const std::shared_ptr<GeometryComponent>& geometry) {
    meta_data &entityData = m_entityData.at(entity);
    std::vector<unsigned int>& faces{geometry->getFaces()};
    size_t geometryFaceSize{sizeof(unsigned int) * faces.size()};
    size_t bufferSize{m_numFaces * sizeof(unsigned int)};

    // create new index buffer with space for the old and new indices;
    unsigned int newEBO{};
    glGenBuffers(1, &newEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newEBO);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize + geometryFaceSize, NULL, GL_DYNAMIC_DRAW);

    // copy the data from the old buffer into the new one
    if (bufferSize) {
        glBindBuffer(GL_COPY_READ_BUFFER, m_EBO);
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ELEMENT_ARRAY_BUFFER, 0, 0, bufferSize);
    }

    glDeleteBuffers(1, &m_EBO);

    m_EBO = newEBO;

    std::get<2>(entityData) = m_numFaces;
    std::get<3>(entityData) = m_numFaces + faces.size();

    m_numFaces += faces.size();
}

void Engine::OpenGLRenderComponent::updateFaces(unsigned int entity, const std::shared_ptr<GeometryComponent>& geometry) {
    // we need to make a copy because we have to offset all face indices by the amount of vertices comping before this object
    std::vector<unsigned int> faces{geometry->getFaces()};
    size_t vertexOffset{std::get<0>(m_entityData.at(entity))};
    for(unsigned int& face: faces) {
        face += vertexOffset;
    }
    size_t objectFaceStart{std::get<2>(m_entityData.at(entity)) * sizeof(unsigned int)};
    // copy the data of the new geometry into the new buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, objectFaceStart, faces.size() * sizeof(unsigned int), faces.data());
}

void Engine::OpenGLRenderComponent::removeFaces(unsigned int entity) {
    unsigned int& objectStart{std::get<2>(m_entityData.at(entity))}; // face index at which the object starts
    unsigned int& objectEnd{std::get<3>(m_entityData.at(entity))}; // face index ath which the object starts
    unsigned int numObjectFaces{objectEnd - objectStart};

    size_t objectSize{sizeof(unsigned int) * numObjectFaces};
    size_t oldBufferSize{m_numFaces * sizeof(unsigned int)};
    size_t newBufferSize{oldBufferSize - objectSize};

    glBindVertexArray(m_VAO);
    // create smaller index buffer and copy info of all other objects into it
    unsigned int newEBO{};
    glGenBuffers(1, &newEBO);
    glBindBuffer(GL_COPY_READ_BUFFER, m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, newBufferSize, NULL, GL_DYNAMIC_DRAW);

    size_t firstBlockSize{sizeof(unsigned int) * objectStart}; // the size of the old buffer before the object to remove
    size_t secondBlockSize{sizeof(unsigned int) * (m_numFaces - objectEnd)}; // the size of the old buffer behind the object to remove

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ELEMENT_ARRAY_BUFFER, 0, 0, firstBlockSize);
    // skip over the object to delete in old buffer
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ELEMENT_ARRAY_BUFFER, firstBlockSize + objectSize, firstBlockSize, secondBlockSize);

    glDeleteBuffers(1, &m_EBO);
    m_EBO = newEBO;

    objectStart = 0;
    objectEnd = 0;

    m_numFaces -= numObjectFaces;
}

void Engine::OpenGLRenderComponent::calculatePrimitiveCount() {
    if (m_primitiveType == GL_POINTS) {
        m_numPrimitives = m_numPoints;
    } else {
        m_numPrimitives = m_numFaces;
    }
}

void Engine::OpenGLRenderComponent::updateMaterialIndices(unsigned int entity) {
    meta_data &entityData = m_entityData.at(entity);
    size_t numVertices{std::get<1>(entityData) - std::get<0>(entityData)};
    float matIndex{std::get<4>(entityData)};
    size_t objectStart{std::get<0>(entityData) * stride};

    for (int i = 0; i < numVertices; ++i) 
    {  
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, objectStart + stride * i + matIndexOffset, sizeof(float), &matIndex);
    }    
}

void Engine::OpenGLRenderComponent::updateTransformIndices(unsigned int entity) {
    meta_data &entityData = m_entityData.at(entity);
    size_t numVertices{std::get<1>(entityData) - std::get<0>(entityData)};
    float transIndex{std::get<5>(entityData)};
    size_t objectStart{std::get<0>(entityData) * stride};

    for (int i = 0; i < numVertices; ++i) 
    {  
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferSubData(GL_ARRAY_BUFFER, objectStart + stride * i + transIndexOffset, sizeof(float), &transIndex);
    }
}

void Engine::OpenGLRenderComponent::updatePrimitiveType(int primitiveType) {
    m_primitiveType = primitiveType;
    calculatePrimitiveCount();
}

void Engine::OpenGLRenderComponent::addMaterial(unsigned int entityId, const std::shared_ptr<MaterialComponent>& material) {
    const std::list<unsigned int>& owners = m_registry.getOwners<MaterialComponent>(material);
    float& matIndex = std::get<4>(m_entityData.at(entityId));

    // handle removal of the material from the given entity (use standard material)
    std::get<9>(m_entityData.at(entityId)) = m_registry.onRemove<MaterialComponent>([=](unsigned int removeEntity, std::weak_ptr<MaterialComponent> removedMaterial) {
        if (removeEntity == entityId) {
            this->removeMaterial(removeEntity, removedMaterial.lock());
        }
    });

    for (unsigned int owner: owners) {
        if (m_entityData.find(owner) != m_entityData.end()) {
            float ownerMatIndex = std::get<4>(m_entityData.at(owner));
            if (ownerMatIndex) {
                // material already known; just copy index
                matIndex = ownerMatIndex;
                updateMaterialIndices(entityId);
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

    updateMaterial(entityId, material);

    // subscribe to material updates
    std::get<8>(m_entityData.at(entityId)) = m_registry.onUpdate<MaterialComponent>(entityId, [this, entityId] (unsigned int updateEntity, std::weak_ptr<MaterialComponent> updatedMaterial) {
        this->updateMaterial(entityId, updatedMaterial.lock());
    });

    updateMaterialIndices(entityId);

    ++m_numMaterials;
}

void Engine::OpenGLRenderComponent::removeMaterial(unsigned int entityId, const std::shared_ptr<MaterialComponent>& material) {
    meta_data& entityData = m_entityData.at(entityId);
    unsigned int oldMatIndex = std::get<4>(entityData);

    // use standard material
    std::get<4>(entityData) = 0;
    updateMaterialIndices(entityId);

    bool holdsOnUpdate{std::get<8>(entityData)};

    // wait for new material to be added
    std::get<8>(entityData) = m_registry.onAdded<MaterialComponent>([=](unsigned int addEntity, std::weak_ptr<MaterialComponent> addedMaterial) {
        if (addEntity == entityId) {
            std::get<8>(this->m_entityData.at(addEntity)).reset();
            this->addMaterial(addEntity, addedMaterial.lock());
        }
    });

    if (holdsOnUpdate) {
        // entity "manages the material" 
        const std::list<unsigned int>& owners{ m_registry.getOwners<MaterialComponent>(material)};
        for (unsigned int owner: owners) {
            // there is another entity using the material 
            if (owner != entityId && m_entityData.find(owner) != m_entityData.end()) {
                // make the other entity handle the updates
                std::get<8>(m_entityData.at(owner)) = m_registry.onUpdate<MaterialComponent>(owner, [=](unsigned int updateEntity, std::weak_ptr<MaterialComponent> updatedMaterial) {
                    this->updateMaterial(owner, updatedMaterial.lock());
                });
                return;
            }
        }

        // there are no other entities that need this material -> remove material from buffer and update 
        unsigned int newMaterialUBO{};
        glGenBuffers(1, &newMaterialUBO);
        glBindBuffer(GL_COPY_READ_BUFFER, m_materialUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, newMaterialUBO);
        glBufferData(GL_UNIFORM_BUFFER, (m_numMaterials - 1) * sizeof(Material), NULL, GL_DYNAMIC_DRAW);
        size_t firstBlockSize{oldMatIndex * sizeof(Material)};
        size_t secondBlockSize{((m_numMaterials - 1) - oldMatIndex) * sizeof(Material)};
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_UNIFORM_BUFFER, 0, 0, firstBlockSize);
        // skip over the material to delete in old buffer
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_UNIFORM_BUFFER, firstBlockSize + sizeof(Material), firstBlockSize, secondBlockSize);
        
        glUniformBlockBinding(m_program.getProgram(), m_program.getBlockIndex("Materials"), 0);

        glDeleteBuffers(1, &m_materialUBO);
        m_materialUBO = newMaterialUBO;

        for (auto& entry: m_entityData) {
            float& matIndex{std::get<4>(entry.second)};
            if (matIndex > oldMatIndex) {
                --matIndex;
                updateMaterialIndices(entry.first);
            }
        }

        --m_numMaterials;
    }    
}

void Engine::OpenGLRenderComponent::updateMaterial(unsigned int entity, const std::shared_ptr<MaterialComponent>& material) {
    glBindBuffer(GL_UNIFORM_BUFFER, this->m_materialUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, std::get<4>(this->m_entityData.at(entity)) * sizeof(Material), sizeof(Material), material->getColor().raw());
}

void Engine::OpenGLRenderComponent::addTransform(unsigned int entityId, const std::shared_ptr<TransformComponent>& transform) {
    const std::list<unsigned int>& owners = m_registry.getOwners<TransformComponent>(transform);
    float& transIndex = std::get<5>(m_entityData.at(entityId));

    // handle removal of the transform from the given entity (use standard transform)
    std::get<11>(m_entityData.at(entityId)) = m_registry.onRemove<TransformComponent>([=](unsigned int removeEntity, std::weak_ptr<TransformComponent> removedTransform) {
        if (removeEntity == entityId) {
            this->removeTransform(removeEntity, removedTransform.lock());
        }
    });

    for (unsigned int owner: owners) {
        if (m_entityData.find(owner) != m_entityData.end()) {
            float ownerTransIndex = std::get<5>(m_entityData.at(owner));
            if (ownerTransIndex) {
                // transform already known; just copy index
                transIndex = ownerTransIndex;
                updateTransformIndices(entityId);
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
    glBufferData(GL_UNIFORM_BUFFER, (m_numTransforms + 1) * sizeof(baseTransform), NULL, GL_DYNAMIC_DRAW);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_UNIFORM_BUFFER, 0, 0, m_numTransforms * sizeof(baseTransform));
    
    glUniformBlockBinding(m_program.getProgram(), m_program.getBlockIndex("Transforms"), 1);

    glDeleteBuffers(1, &m_transformUBO);
    m_transformUBO = newTransformUBO;

    updateTransform(entityId, transform);

    // subscribe to transform updates
    std::get<10>(m_entityData.at(entityId)) = m_registry.onUpdate<TransformComponent>(entityId, [this, entityId] (unsigned int updateEntity, std::weak_ptr<TransformComponent> updatedTransform) {
        this->updateTransform(entityId, updatedTransform.lock());
    });

    updateTransformIndices(entityId);

    ++m_numTransforms;
}
const size_t matrixSize{ 16 * sizeof(float) };
void Engine::OpenGLRenderComponent::updateTransform(unsigned int entity, const std::shared_ptr<TransformComponent>& transform) {
    glBindBuffer(GL_UNIFORM_BUFFER, this->m_transformUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, std::get<5>(this->m_entityData.at(entity)) * sizeof(baseTransform), matrixSize, transform->getModelMatrix().raw());
    glBufferSubData(GL_UNIFORM_BUFFER, std::get<5>(this->m_entityData.at(entity)) * sizeof(baseTransform) + matrixSize, matrixSize, transform->getNormalMatrix().raw());
}

void Engine::OpenGLRenderComponent::removeTransform(unsigned int entity, const std::shared_ptr<TransformComponent>& transform) {
    meta_data& entityData = m_entityData.at(entity);
    unsigned int oldTransIndex = std::get<5>(entityData);

    // use standard transform
    std::get<5>(entityData) = 0;
    updateTransformIndices(entity);

    bool holdsOnUpdate{std::get<10>(entityData)};

    // wait for new transform to be added
    std::get<10>(entityData) = m_registry.onAdded<TransformComponent>([=](unsigned int addEntity, std::weak_ptr<TransformComponent> addedTransform) {
        if (addEntity == entity) {
            std::get<10>(this->m_entityData.at(addEntity)).reset();
            this->addTransform(addEntity, addedTransform.lock());
        }
    });

    if (holdsOnUpdate) {
        // entity "manages the transform" 
        const std::list<unsigned int>& owners{ m_registry.getOwners<TransformComponent>(transform)};
        for (unsigned int owner: owners) {
            // there is another entity using the transform 
            if (owner != entity && m_entityData.find(owner) != m_entityData.end()) {
                // make the other entity handle the updates
                std::get<10>(m_entityData.at(owner)) = m_registry.onUpdate<TransformComponent>(owner, [=](unsigned int updateEntity, std::weak_ptr<TransformComponent> updatedTransform) {
                    this->updateTransform(owner, updatedTransform.lock());
                });
                return;
            }
        }

        // there are no other entities that need this transform -> remove transform from buffer and update 
        unsigned int newTransformUBO{};
        glGenBuffers(1, &newTransformUBO);
        glBindBuffer(GL_COPY_READ_BUFFER, m_transformUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, newTransformUBO);
        glBufferData(GL_UNIFORM_BUFFER, (m_numTransforms - 1) * sizeof(baseTransform), NULL, GL_DYNAMIC_DRAW);
        size_t firstBlockSize{oldTransIndex * sizeof(baseTransform)};
        size_t secondBlockSize{((m_numTransforms - 1) - oldTransIndex) * sizeof(baseTransform)};
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_UNIFORM_BUFFER, 0, 0, firstBlockSize);
        // skip over the transform to delete in old buffer
        glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_UNIFORM_BUFFER, firstBlockSize + sizeof(baseTransform), firstBlockSize, secondBlockSize);
        
        glUniformBlockBinding(m_program.getProgram(), m_program.getBlockIndex("Transforms"), 1);

        glDeleteBuffers(1, &m_transformUBO);
        m_transformUBO = newTransformUBO;

        for (auto& entry: m_entityData) {
            float& transIndex{std::get<5>(entry.second)};
            if (transIndex > oldTransIndex) {
                --transIndex;
                updateTransformIndices(entry.first);
            }
        }

        --m_numTransforms;
    }    
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
            std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<GeometryComponent>)>>, // callback for when a GeometryComponent is added or updated to the entity
            std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<GeometryComponent>)>>, // callback for when a GeometryComponent is removed from the entity
            std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<MaterialComponent>)>>, // callback for when a MaterialComponent is added or updated to the entity
            std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<MaterialComponent>)>>, // callback for when a MaterialComponent is removed from the entity
            std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>>, // callback for when a MaterialComponent is added or updated to the entity
            std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>>,  // callback for when a MaterialComponent is removed from the entity
            std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<OpenGLRenderComponent>)>> // callback for when the Render component is removed from the entity
        >{
        -1, -1, -1, -1, 0.0, 0.0,
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<GeometryComponent>)>>{},
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<GeometryComponent>)>>{},
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<MaterialComponent>)>>{},
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<MaterialComponent>)>>{},
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>>{},
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>>{}, 
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<OpenGLRenderComponent>)>>{}
    });

    std::shared_ptr<GeometryComponent> geometry = m_registry.getComponent<Engine::GeometryComponent>(entity);
    if (geometry) {
        // entity has a geometry component => setup buffers to use geometry data of that component
        setupEntity(entity, geometry);
    } else {
        // entity has no geometry; setup callback and postpone setup until it gets one
        std::get<6>(m_entityData.at(entity)) = m_registry.onAdded<GeometryComponent>([=](unsigned int addEntity, std::weak_ptr<GeometryComponent> geometry) {
            if (addEntity == entity) {
                this->setupEntity(entity, geometry.lock());
            }
        });
    }

    // callback that removes all information for the entity if the renderer component is removed from the entity
    std::get<12>(m_entityData.at(entity)) = m_registry.onRemove<OpenGLRenderComponent>([=](unsigned int removeEntity, std::weak_ptr<OpenGLRenderComponent> render) {
        if (removeEntity == entity) {
            this->dissassociate(entity);
        }
    });
}

void Engine::OpenGLRenderComponent::setupEntity(unsigned int entity, const std::shared_ptr<GeometryComponent>& geometry) {
    addVertices(entity, geometry);
    addFaces(entity, geometry);
    updateVertices(entity, geometry);
    updateFaces(entity, geometry);
    calculatePrimitiveCount();


    // subscribe for updates on the geometry to update the buffer data accordingly
    std::get<6>(m_entityData.at(entity)) = m_registry.onUpdate<GeometryComponent>(entity, [=](unsigned int updateEntity, std::weak_ptr<GeometryComponent> updatedGeometry) {
        unsigned int numVertices{std::get<1>(this->m_entityData.at(updateEntity)) - std::get<0>(this->m_entityData.at(updateEntity))};
        std::shared_ptr<GeometryComponent> updated{updatedGeometry.lock()};


        if (numVertices != updated->getVertices().size()) {
            removeVertices(updateEntity);
            addVertices(updateEntity, updated);
            updateMaterialIndices(entity);
            updateTransformIndices(entity);
        }  

        unsigned int numFaces{std::get<3>(this->m_entityData.at(updateEntity)) - std::get<2>(this->m_entityData.at(updateEntity))};

        if (numFaces != updated->getFaces().size()) {
            removeFaces(updateEntity);
            addFaces(updateEntity, updated);
        }  

        updateVertices(updateEntity, updated);
        updateFaces(updateEntity, updated);
        calculatePrimitiveCount();
    });

    std::shared_ptr<MaterialComponent> material = m_registry.getComponent<Engine::MaterialComponent>(entity);
    if (material) {
        addMaterial(entity, material);
    } else {
        std::get<8>(m_entityData.at(entity)) = m_registry.onAdded<MaterialComponent>([=](unsigned int addEntity, std::weak_ptr<MaterialComponent> material) {
           if (addEntity == entity) {
               addMaterial(entity, material.lock());
           } 
        });
    }

    std::shared_ptr<TransformComponent> transform = m_registry.getComponent<TransformComponent>(entity);
    if (transform) {
        addTransform(entity, transform);
    } else {
        std::get<10>(m_entityData.at(entity)) = m_registry.onAdded<TransformComponent>([=](unsigned int addEntity, std::weak_ptr<TransformComponent> transform) {
           if (addEntity == entity) {
               addTransform(entity, transform.lock());
           } 
        });
    }

    updateMaterialIndices(entity);
    updateTransformIndices(entity);

    std::get<7>(m_entityData.at(entity)) = m_registry.onRemove<GeometryComponent>([=](unsigned int removeEntity, std::weak_ptr<GeometryComponent> removeGeometry) {
        if (entity == removeEntity) {
            removeVertices(removeEntity);
            removeFaces(removeEntity);
            calculatePrimitiveCount();
            // entity has no geometry anymore; setup callback
            std::get<6>(m_entityData.at(removeEntity)) = m_registry.onAdded<GeometryComponent>([=](unsigned int addEntity, std::weak_ptr<GeometryComponent> addGeometry) {
                if (addEntity == removeEntity) {
                    std::shared_ptr<GeometryComponent> added{addGeometry.lock()};
                    addVertices(addEntity, added);
                    addFaces(addEntity, added);
                    updateVertices(addEntity, added);
                    updateFaces(addEntity, added);
                    calculatePrimitiveCount();
                    updateMaterialIndices(addEntity);
                    updateTransformIndices(addEntity);
                }
            });
        }
    }); 
}

void Engine::OpenGLRenderComponent::teardownEntity(unsigned int entity) {
    meta_data& entityData{m_entityData.at(entity)};
    // entity "manages" material => assign management to another entity or remove
    if (std::get<4>(entityData) && std::get<8>(entityData)) {
        removeMaterial(entity, m_registry.getComponent<MaterialComponent>(entity));
    }

    // entity "manages" transform => assign management to another entity or remove
    if (std::get<5>(entityData) && std::get<10>(entityData)) {
        removeTransform(entity, m_registry.getComponent<TransformComponent>(entity));
    }

    removeVertices(entity);
    removeFaces(entity);
}

void Engine::OpenGLRenderComponent::dissassociate(unsigned int entity) {
    teardownEntity(entity);

    m_entityData.erase(entity);
}

void Engine::OpenGLRenderComponent::render() {
    m_program.use();

    glBindVertexArray(m_VAO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_materialUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_transformUBO);

    if (m_primitiveType == GL_POINTS) {
        glDrawArrays(GL_POINTS, 0, m_numPrimitives);
    } else {
        glDrawElements(m_primitiveType, m_numPrimitives, GL_UNSIGNED_INT, 0);
    }
}