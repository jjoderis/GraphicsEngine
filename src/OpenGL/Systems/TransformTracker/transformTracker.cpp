#include "transformTracker.h"

#include "../../../Core/Components/Transform/transform.h"
#include "../../../Core/ECS/registry.h"
#include "../../Components/OpenGLTransform/openGLTransform.h"

Engine::Systems::OpenGLTransformTracker::OpenGLTransformTracker(Registry &registry) : m_registry{registry}
{
    m_updateCallback = m_registry.onUpdate<Engine::TransformComponent>(
        [&](unsigned int updateEntity, std::weak_ptr<TransformComponent> transform)
        { update(updateEntity, transform.lock().get()); });

    m_removeCallback = m_registry.onRemove<Engine::TransformComponent>(
        [&](unsigned int removeEntity, std::weak_ptr<TransformComponent> transform)
        { update(removeEntity, transform.lock().get()); });
}

void Engine::Systems::OpenGLTransformTracker::update(unsigned int entity, TransformComponent *transform)
{
    // we don't expect entities to share TransformComponents
    if (auto openGLComponent = m_registry.getComponent<Engine::OpenGLTransformComponent>(entity))
    {
        openGLComponent->update(transform);
    }
}

void Engine::Systems::OpenGLTransformTracker::remove(unsigned int entity)
{
    m_registry.removeComponent<Engine::OpenGLTransformComponent>(entity);
}