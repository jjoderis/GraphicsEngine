#include "lightsTracker.h"

template <typename LightType>
Engine::Systems::OpenGLLightsTracker<LightType>::OpenGLLightsTracker(unsigned int &lightsUBO, Registry &registry)
    : m_lightsUBO{lightsUBO}, m_registry{registry}
{
    glGenBuffers(1, &m_lightsUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUBO);

    glBufferData(GL_UNIFORM_BUFFER, 4 * sizeof(int), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(int), &m_numLights);

    m_AddLightCB = m_registry.onAdded<LightType>(
        [this](unsigned int entity, std::weak_ptr<LightType> light)
        {
            // if the entity wasn't associated with this component before
            if (m_entityData.find(entity) == m_entityData.end())
            {
                m_entityData.emplace(
                    entity,
                    meta_data{0,
                              std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<LightType>)>>{},
                              std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<LightType>)>>{},
                              std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>>{},
                              std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>>{}});

                addLight(entity, light.lock());
                resetTransformInfo(entity);
                awaitTransform(entity);
            }
        });
}

template <typename LightType>
size_t Engine::Systems::OpenGLLightsTracker<LightType>::getLightInfoSize()
{
    // we need actual specialization for the specific light types
    return 0;
}

template <>
size_t Engine::Systems::OpenGLLightsTracker<Engine::AmbientLightComponent>::getLightInfoSize()
{
    // we have only a light color (3 * size of float), buffer layout pads to size of 4 floats
    return 4 * sizeof(float);
}

template <>
size_t Engine::Systems::OpenGLLightsTracker<Engine::DirectionalLightComponent>::getLightInfoSize()
{
    // we have direction (3 * size of float) + color (3 * size of float), buffer layout padds to size of 4 floats
    return 8 * sizeof(float);
}

template <>
size_t Engine::Systems::OpenGLLightsTracker<Engine::PointLightComponent>::getLightInfoSize()
{
    // we have intensity (1 * size of float) + position (3 * size of float) + color (3 * size of float), buffer layout
    // padds to size of 4 floats
    return 12 * sizeof(float);
}

template <>
size_t Engine::Systems::OpenGLLightsTracker<Engine::SpotLightComponent>::getLightInfoSize()
{
    // we have intensity (1 * size of float) + cutoff angle (1 * size of float) + penumbra angle (1 * size of float) +
    // position (3 * size of float) + direction (3 * size of float) + color (3 * size of float), buffer layout padds to
    // size of 4 floats
    return 16 * sizeof(float);
}

template <typename LightType>
void Engine::Systems::OpenGLLightsTracker<LightType>::addLight(unsigned int entity,
                                                               const std::shared_ptr<LightType> &light)
{
    size_t lightInfoSize{getLightInfoSize()};

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

    glDeleteBuffers(1, &m_lightsUBO);
    m_lightsUBO = newUBO;

    meta_data &entityData{m_entityData.at(entity)};

    std::get<0>(entityData) = offset;

    // setup callbacks for changes in entity light
    std::get<1>(entityData) =
        m_registry.onUpdate<LightType>(entity,
                                       [this, entity](unsigned int updateEntity, std::weak_ptr<LightType> updatedLight)
                                       { this->updateLightInfo(entity, updatedLight.lock()); });
    std::get<2>(entityData) = m_registry.onRemove<LightType>(
        [this, entity](unsigned int removeEntity, std::weak_ptr<LightType> light)
        {
            if (removeEntity == entity)
            {
                this->removeLight(entity);
            }
        });

    updateLightInfo(entity, light);
}

template <typename LightType>
void Engine::Systems::OpenGLLightsTracker<LightType>::updateLightInfo(unsigned int entity,
                                                                      const std::shared_ptr<LightType> &light)
{
    // actual implementation will have to be done as specalization for each of the light types
}

template <>
void Engine::Systems::OpenGLLightsTracker<Engine::AmbientLightComponent>::updateLightInfo(
    unsigned int entity, const std::shared_ptr<AmbientLightComponent> &light)
{
    size_t offset{std::get<0>(m_entityData.at(entity))};

    glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUBO);
    // add color
    glBufferSubData(GL_UNIFORM_BUFFER, offset, 3 * sizeof(float), light->getColor().raw());
}

template <>
void Engine::Systems::OpenGLLightsTracker<Engine::DirectionalLightComponent>::updateLightInfo(
    unsigned int entity, const std::shared_ptr<DirectionalLightComponent> &light)
{
    size_t offset{std::get<0>(m_entityData.at(entity))};

    glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUBO);
    // add direction
    glBufferSubData(GL_UNIFORM_BUFFER, offset, 3 * sizeof(float), light->getDirection().raw());
    // add color (vec3s are padded to the size of vec4 leading to the offset)
    glBufferSubData(GL_UNIFORM_BUFFER, offset + 4 * sizeof(float), 3 * sizeof(float), light->getColor().raw());
}

template <>
void Engine::Systems::OpenGLLightsTracker<Engine::PointLightComponent>::updateLightInfo(
    unsigned int entity, const std::shared_ptr<PointLightComponent> &light)
{
    size_t offset{std::get<0>(m_entityData.at(entity))};

    glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUBO);
    // add intensity
    float intensity{light->getIntensity()};
    glBufferSubData(GL_UNIFORM_BUFFER, offset, 1 * sizeof(float), &intensity);
    // add position
    glBufferSubData(GL_UNIFORM_BUFFER, offset + 4 * sizeof(float), 3 * sizeof(float), light->getPosition().raw());
    // add color (vec3s are padded to the size of vec4 leading to the offset)
    glBufferSubData(GL_UNIFORM_BUFFER, offset + 8 * sizeof(float), 3 * sizeof(float), light->getColor().raw());
}

template <>
void Engine::Systems::OpenGLLightsTracker<Engine::SpotLightComponent>::updateLightInfo(
    unsigned int entity, const std::shared_ptr<SpotLightComponent> &light)
{
    size_t offset{std::get<0>(m_entityData.at(entity))};

    glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUBO);
    // add intensity
    float intensity{light->getIntensity()};
    glBufferSubData(GL_UNIFORM_BUFFER, offset, 1 * sizeof(float), &intensity);
    // add cutoff angle
    float cutoff{light->getCutoff()};
    glBufferSubData(GL_UNIFORM_BUFFER, offset + sizeof(float), 1 * sizeof(float), &cutoff);
    // add penumbra angle
    float penumbra{light->getPenumbra()};
    glBufferSubData(GL_UNIFORM_BUFFER, offset + 2 * sizeof(float), 1 * sizeof(float), &penumbra);
    // add position
    glBufferSubData(GL_UNIFORM_BUFFER, offset + 4 * sizeof(float), 3 * sizeof(float), light->getPosition().raw());
    // add direction
    glBufferSubData(GL_UNIFORM_BUFFER, offset + 8 * sizeof(float), 3 * sizeof(float), light->getDirection().raw());
    // add color (vec3s are padded to the size of vec4 leading to the offset)
    glBufferSubData(GL_UNIFORM_BUFFER, offset + 12 * sizeof(float), 3 * sizeof(float), light->getColor().raw());
}

template <typename LightType>
void Engine::Systems::OpenGLLightsTracker<LightType>::removeLight(unsigned int entity)
{
    size_t lightInfoSize{getLightInfoSize()};

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

    size_t firstBlockSize{objectStart};                // the size of the old buffer before the light to remove
    size_t secondBlockSize{oldBufferSize - objectEnd}; // the size of the old buffer behind the object to remove

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_UNIFORM_BUFFER, 0, 0, firstBlockSize);
    // skip over the object to delete in old buffer
    glCopyBufferSubData(
        GL_COPY_READ_BUFFER, GL_UNIFORM_BUFFER, firstBlockSize + objectSize, firstBlockSize, secondBlockSize);

    glDeleteBuffers(1, &m_lightsUBO);
    m_lightsUBO = newUBO;

    for (auto &entry : m_entityData)
    {
        size_t otherObjectStart{std::get<0>(entry.second)};
        if (entry.first != entity && otherObjectStart > objectStart)
        {
            otherObjectStart -= objectSize;
        }
    }

    m_entityData.erase(entity);

    --m_numLights;

    // update number of lights in buffer
    glBindBuffer(GL_UNIFORM_BUFFER, newUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(int), &m_numLights);
}

template <typename LightType>
void Engine::Systems::OpenGLLightsTracker<LightType>::awaitTransform(unsigned int entity)
{
    std::shared_ptr<TransformComponent> transform{m_registry.getComponent<TransformComponent>(entity)};

    if (transform)
    {
        this->updateTransformInfo(entity, transform);
        std::get<3>(m_entityData.at(entity)) = m_registry.onUpdate<TransformComponent>(
            entity,
            [this](unsigned int updateEntity, std::weak_ptr<TransformComponent> transform)
            { this->updateTransformInfo(updateEntity, transform.lock()); });
        std::get<4>(m_entityData.at(entity)) = m_registry.onRemove<TransformComponent>(
            [this, entity](unsigned int removeEntity, std::weak_ptr<TransformComponent> removedTransform)
            {
                if (entity == removeEntity)
                {
                    this->resetTransformInfo(removeEntity);
                    std::get<3>(m_entityData.at(entity)) = m_registry.onAdded<TransformComponent>(
                        [this, removeEntity](unsigned int addEntity, std::weak_ptr<TransformComponent> addedTransform)
                        {
                            if (removeEntity == addEntity)
                            {
                                this->awaitTransform(addEntity);
                            }
                        });
                }
            });
    }
    else
    {
        resetTransformInfo(entity);
        std::get<3>(m_entityData.at(entity)) = m_registry.onAdded<TransformComponent>(
            [this, entity](unsigned int addEntity, std::weak_ptr<TransformComponent> addedTransform)
            {
                if (entity == addEntity)
                {
                    this->awaitTransform(addEntity);
                }
            });
    }
}

template <typename LightType>
void Engine::Systems::OpenGLLightsTracker<LightType>::resetTransformInfo(unsigned int entity)
{
    size_t offset{std::get<0>(m_entityData.at(entity))};

    const float origin[3]{0.0f, 0.0f, 0.0f};

    glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(origin), origin);
}

template <>
void Engine::Systems::OpenGLLightsTracker<Engine::AmbientLightComponent>::resetTransformInfo(unsigned int entity)
{
    // ambient light is not affected by transformation
}

template <>
void Engine::Systems::OpenGLLightsTracker<Engine::DirectionalLightComponent>::resetTransformInfo(unsigned int entity)
{
    // we expect this component to exist since we only should be tracking existing lights
    std::shared_ptr<DirectionalLightComponent> light{m_registry.getComponent<DirectionalLightComponent>(entity)};

    // let the updateLightInfo function take care of this
    light->setDirection(Math::Vector3{0.0, 0.0, 1.0});
    m_registry.updated<DirectionalLightComponent>(entity);
}

template <>
void Engine::Systems::OpenGLLightsTracker<Engine::PointLightComponent>::resetTransformInfo(unsigned int entity)
{
    // we expect this component to exist since we only should be tracking existing lights
    std::shared_ptr<PointLightComponent> light{m_registry.getComponent<PointLightComponent>(entity)};

    // let the updateLightInfo function take care of this
    light->setPosition(Math::Vector3{0.0, 0.0, 0.0});
    m_registry.updated<PointLightComponent>(entity);
}

template <>
void Engine::Systems::OpenGLLightsTracker<Engine::SpotLightComponent>::resetTransformInfo(unsigned int entity)
{
    // we expect this component to exist since we only should be tracking existing lights
    std::shared_ptr<SpotLightComponent> light{m_registry.getComponent<SpotLightComponent>(entity)};

    // let the updateLightInfo function take care of this
    light->setPosition(Math::Vector3{0.0, 0.0, 0.0});

    light->setDirection(Math::Vector3{0.0, 0.0, 1.0});
    m_registry.updated<SpotLightComponent>(entity);
}

template <typename LightType>
void Engine::Systems::OpenGLLightsTracker<LightType>::updateTransformInfo(
    unsigned int entity, const std::shared_ptr<TransformComponent> &transform)
{
    size_t offset{std::get<0>(m_entityData.at(entity))};

    glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, offset, 3 * sizeof(float), transform->getTranslation().raw());
}

template <>
void Engine::Systems::OpenGLLightsTracker<Engine::AmbientLightComponent>::updateTransformInfo(
    unsigned int entity, const std::shared_ptr<TransformComponent> &transform)
{
    // ambient light is not affected by transformation
}

template <>
void Engine::Systems::OpenGLLightsTracker<Engine::DirectionalLightComponent>::updateTransformInfo(
    unsigned int entity, const std::shared_ptr<TransformComponent> &transform)
{
    // we expect this component to exist since we only should be tracking existing lights
    std::shared_ptr<DirectionalLightComponent> light{m_registry.getComponent<DirectionalLightComponent>(entity)};

    // let the updateLightInfo function take care of this
    light->setDirection(transform->getModelMatrix() * Math::Vector4{0.0, 0.0, 1.0, 0.0});
    m_registry.updated<DirectionalLightComponent>(entity);
}

template <>
void Engine::Systems::OpenGLLightsTracker<Engine::PointLightComponent>::updateTransformInfo(
    unsigned int entity, const std::shared_ptr<TransformComponent> &transform)
{
    // we expect this component to exist since we only should be tracking existing lights
    std::shared_ptr<PointLightComponent> light{m_registry.getComponent<PointLightComponent>(entity)};

    // let the updateLightInfo function take care of this
    light->setPosition(transform->getTranslation());
    m_registry.updated<PointLightComponent>(entity);
}

template <>
void Engine::Systems::OpenGLLightsTracker<Engine::SpotLightComponent>::updateTransformInfo(
    unsigned int entity, const std::shared_ptr<TransformComponent> &transform)
{
    // we expect this component to exist since we only should be tracking existing lights
    std::shared_ptr<SpotLightComponent> light{m_registry.getComponent<SpotLightComponent>(entity)};

    // let the updateLightInfo function take care of this
    light->setPosition(transform->getTranslation());
    light->setDirection(transform->getModelMatrix() * Math::Vector4{0.0, 0.0, 1.0, 0.0});
    m_registry.updated<SpotLightComponent>(entity);
}

template class Engine::Systems::OpenGLLightsTracker<Engine::AmbientLightComponent>;
template class Engine::Systems::OpenGLLightsTracker<Engine::DirectionalLightComponent>;
template class Engine::Systems::OpenGLLightsTracker<Engine::PointLightComponent>;
template class Engine::Systems::OpenGLLightsTracker<Engine::SpotLightComponent>;