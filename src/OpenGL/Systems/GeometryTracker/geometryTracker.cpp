#include "geometryTracker.h"

#include "../../../Core/Components/Geometry/geometry.h"
#include "../../../Core/ECS/registry.h"
#include "../../Components/OpenGLGeometry/openGLGeometry.h"
#include <set>

Engine::Systems::OpenGLGeometryTracker::OpenGLGeometryTracker(Registry &registry) : m_registry{registry}
{
    m_updateCallback = m_registry.onUpdate<Engine::GeometryComponent>(
        [&](unsigned int updateEntity, std::weak_ptr<GeometryComponent> geometry)
        { update(updateEntity, geometry.lock().get()); });

    m_removeCallback = m_registry.onRemove<Engine::GeometryComponent>(
        [&](unsigned int removeEntity, std::weak_ptr<GeometryComponent> geometry)
        { remove(removeEntity, geometry.lock().get()); });

    m_swapCallback = m_registry.onComponentSwap<Engine::GeometryComponent>(
        [&](unsigned int swapEntity, std::weak_ptr<GeometryComponent> newGeometry)
        { update(swapEntity, newGeometry.lock().get()); });
}

void Engine::Systems::OpenGLGeometryTracker::update(unsigned int entity, GeometryComponent *geometry)
{
    // get the Owners of the GeometryComponent
    auto &owners{m_registry.getOwners<GeometryComponent>(entity)};

    // prevent multiple updates of same OpenGLGeometryComponent on different owners
    std::set<OpenGLGeometryComponent *> uniqueOpenGLComponents{};
    for (unsigned int owner : owners)
    {
        if (auto openGLComponent = m_registry.getComponent<Engine::OpenGLGeometryComponent>(owner))
        {
            uniqueOpenGLComponents.emplace(openGLComponent.get());
        }
    }

    for (auto &openGLComponent : uniqueOpenGLComponents)
    {
        openGLComponent->update(geometry);
    }
}

void Engine::Systems::OpenGLGeometryTracker::remove(unsigned int entity, GeometryComponent *geometry)
{
    m_registry.removeComponent<Engine::OpenGLGeometryComponent>(entity);
}