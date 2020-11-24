#include "lightsTracker.h"

Engine::Systems::OpenGLLightsTracker::OpenGLLightsTracker(unsigned int& lightsUBO, Registry& registry) 
    : m_lightsUBO{lightsUBO}, m_registry{registry}
{
    glGenBuffers(1, &m_lightsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUBO);

    glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(int), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(int), &m_numLights);

    m_AddLightCB = m_registry.onAdded<LightComponent>([this](unsigned int entity, std::weak_ptr<LightComponent> light) {
        // if the entity wasn't associated with this component before
        if (m_entityData.find(entity) == m_entityData.end()) {
            m_entityData.emplace(entity, meta_data{
                0,
                std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<LightComponent>)>>{},
                std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<LightComponent>)>>{},
                std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>>{},
                std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>>{}
            });

            addLight(entity, light.lock());
        }
    });
}

const size_t lightInfoSize{3 * sizeof(float)};

void Engine::Systems::OpenGLLightsTracker::addLight(unsigned int entity, const std::shared_ptr<LightComponent>& light) {
    size_t offset{4 * sizeof(int) + m_numLights * lightInfoSize};

    unsigned int newUBO{};
    glGenBuffers(1, &newUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, newUBO);
    // create buffer with enough space to store all current data and the data for the new light
    // current data: 1 entry for how many light there are, 1 position per light
    glBufferData(GL_UNIFORM_BUFFER, offset + lightInfoSize, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_COPY_READ_BUFFER, m_lightsUBO);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_UNIFORM_BUFFER, 0, 0, offset);

    ++m_numLights;

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(int), &m_numLights);

    // initialize light info with base transform
    float baseTransform[3]{0.0f, 0.0f, 0.0f};
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(baseTransform), baseTransform);

    glDeleteBuffers(1, &m_lightsUBO);
    m_lightsUBO = newUBO;

    meta_data& entityData{m_entityData.at(entity)};

    std::get<0>(entityData) = offset;
    
    // setup callbacks for changes in entity light
    std::get<1>(entityData) = m_registry.onUpdate<LightComponent>(entity, [this](unsigned int updateEntity, std::weak_ptr<LightComponent> updatedlight) {
        // TODO: update light info in buffer
    });
    std::get<2>(entityData) = m_registry.onRemove<LightComponent>([this, entity](unsigned int removeEntity, std::weak_ptr<LightComponent> light) {
        if (removeEntity == entity) {
            this->removeLight(entity);
        }
    });

    awaitTransform(entity);
}

void Engine::Systems::OpenGLLightsTracker::removeLight(unsigned int entity) {
    size_t objectStart{std::get<0>(m_entityData.at(entity))};
    size_t objectEnd{objectStart + lightInfoSize};

    size_t objectSize{objectEnd - objectStart};
    size_t oldBufferSize{4 * sizeof(int) + m_numLights * lightInfoSize};
    size_t newBufferSize{oldBufferSize - objectSize};

    unsigned int newUBO{};
    glGenBuffers(1, &newUBO);
    glBindBuffer(GL_COPY_READ_BUFFER, m_lightsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, newUBO);
    glBufferData(GL_UNIFORM_BUFFER, newBufferSize, NULL, GL_DYNAMIC_DRAW);

    size_t firstBlockSize{objectStart}; // the size of the old buffer before the light to remove
    size_t secondBlockSize{oldBufferSize - objectEnd}; // the size of the old buffer behind the object to remove

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_UNIFORM_BUFFER, 0, 0, firstBlockSize);
    // skip over the object to delete in old buffer
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_UNIFORM_BUFFER, firstBlockSize + objectSize, firstBlockSize, secondBlockSize);

    glDeleteBuffers(1, &m_lightsUBO);
    m_lightsUBO = newUBO;

    for (auto& entry: m_entityData) {
        size_t otherObjectStart{std::get<0>(entry.second)};
        if (entry.first != entity && otherObjectStart > objectStart) {
            otherObjectStart -= objectSize;
        }
    }

    m_entityData.erase(entity);

    --m_numLights;
}

void Engine::Systems::OpenGLLightsTracker::awaitTransform(unsigned int entity) {
    std::shared_ptr<TransformComponent> transform{ m_registry.getComponent<TransformComponent>(entity) };

    if(transform) {
        this->updateTransformInfo(entity, transform);
        std::get<3>(m_entityData.at(entity)) = m_registry.onUpdate<TransformComponent>(entity, [this](unsigned int updateEntity, std::weak_ptr<TransformComponent> transform) {
            this->updateTransformInfo(updateEntity, transform.lock());
        });
        std::get<4>(m_entityData.at(entity)) = m_registry.onRemove<TransformComponent>([this, entity](unsigned int removeEntity, std::weak_ptr<TransformComponent> removedTransform) {
            if (entity == removeEntity) {
                this->resetTransformInfo(removeEntity);
                std::get<3>(m_entityData.at(entity)) = m_registry.onAdded<TransformComponent>([this, removeEntity](unsigned int addEntity, std::weak_ptr<TransformComponent> addedTransform) {
                    if (removeEntity == addEntity) {
                        this->awaitTransform(addEntity);
                    }
                });
            }
        });
    } else {
        resetTransformInfo(entity);
        std::get<3>(m_entityData.at(entity)) = m_registry.onAdded<TransformComponent>([this, entity](unsigned int addEntity, std::weak_ptr<TransformComponent> addedTransform) {
            if (entity == addEntity) {
                this->awaitTransform(addEntity);
            }
        });
    }
}

void Engine::Systems::OpenGLLightsTracker::resetTransformInfo(unsigned int entity) {
    size_t offset{ std::get<0>(m_entityData.at(entity)) };

    const float origin[3]{ 1.0f, 0.0f, 0.0f };

    glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, 3 * sizeof(float), origin);
}

void Engine::Systems::OpenGLLightsTracker::updateTransformInfo(unsigned int entity, const std::shared_ptr<TransformComponent>& transform) {
    size_t offset{ std::get<0>(m_entityData.at(entity)) };

    glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, 3 * sizeof(float), transform->getTranslation().raw());
}