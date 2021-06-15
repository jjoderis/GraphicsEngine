#include "renderTracker.h"

#include "../../../Core/Components/Geometry/geometry.h"
#include "../../../Core/Components/Hierarchy/hierarchy.h"
#include "../../../Core/Components/Render/render.h"
#include "../../../Core/Components/Transform/transform.h"
#include "../../../Core/ECS/registry.h"
#include "../../Components/Material/material.h"
#include "../../Components/OpenGLGeometry/openGLGeometry.h"
#include "../../Components/OpenGLTransform/openGLTransform.h"
#include "../../Components/Shader/shader.h"
#include "../../Components/Texture/texture.h"

Engine::Systems::OpenGLRenderTracker::OpenGLRenderTracker(Registry &registry) : m_registry{registry}
{
    m_addCallback = m_registry.onAdded<RenderComponent>(
        [this](unsigned int entity, std::weak_ptr<RenderComponent> render) { this->makeRenderable(entity); });
}

void Engine::Systems::OpenGLRenderTracker::makeRenderable(unsigned int entity)
{
    ensureMaterial(entity);

    ensureGeometry(entity);

    ensureTransform(entity);

    // TODO: texture not always necessary
    ensureTexture(entity);

    // render children
    if (auto hierarchy = m_registry.getComponent<Engine::HierarchyComponent>(entity))
    {
        for (unsigned int child : hierarchy->getChildren())
        {
            m_registry.addComponent<Engine::RenderComponent>(child, std::make_shared<Engine::RenderComponent>());
        }
    }
}

void Engine::Systems::OpenGLRenderTracker::ensureGeometry(unsigned int entity)
{
    std::shared_ptr<GeometryComponent> geometry{m_registry.getComponent<GeometryComponent>(entity)};
    if (!geometry)
    {
        geometry = m_registry.addComponent<GeometryComponent>(entity, std::make_shared<Engine::GeometryComponent>());
    }

    // make sure the entity has a OpenGLGeometryComponent
    if (!m_registry.hasComponent<Engine::OpenGLGeometryComponent>(entity))
    {

        std::shared_ptr<Engine::OpenGLGeometryComponent> openGLGeometry;

        // check if another owner of the geometry has a fitting OpenGLGeometryComponent
        // TODO: check if the existing OpenGLGeometry provides data needed for Shader
        for (unsigned int owner : m_registry.getOwners<Engine::GeometryComponent>(entity))
        {
            if (auto existingOpenGLGeometry = m_registry.getComponent<Engine::OpenGLGeometryComponent>(owner))
            {
                openGLGeometry = existingOpenGLGeometry;
                break;
            }
        }

        if (!openGLGeometry)
        {
            openGLGeometry = std::make_shared<Engine::OpenGLGeometryComponent>(geometry.get());
        }

        m_registry.addComponent<Engine::OpenGLGeometryComponent>(entity, openGLGeometry);
    }
}

void Engine::Systems::OpenGLRenderTracker::ensureMaterial(unsigned int entity)
{
    // make sure we have every necessary component to render
    std::shared_ptr<OpenGLMaterialComponent> material{m_registry.getComponent<OpenGLMaterialComponent>(entity)};
    if (!material)
    {
        material =
            m_registry.addComponent<OpenGLMaterialComponent>(entity, std::make_shared<OpenGLMaterialComponent>());
    }
}

void Engine::Systems::OpenGLRenderTracker::ensureTransform(unsigned int entity)
{
    std::shared_ptr<TransformComponent> transform{m_registry.getComponent<TransformComponent>(entity)};
    if (!transform)
    {
        transform = m_registry.addComponent<TransformComponent>(entity, std::make_shared<TransformComponent>());
    }

    if (!m_registry.hasComponent<Engine::OpenGLTransformComponent>(entity))
    {
        m_registry.addComponent<Engine::OpenGLTransformComponent>(
            entity, std::make_shared<Engine::OpenGLTransformComponent>(transform.get()));
    }
}

void Engine::Systems::OpenGLRenderTracker::ensureTexture(unsigned int entity)
{
    std::shared_ptr<OpenGLTextureComponent> texture{m_registry.getComponent<OpenGLTextureComponent>(entity)};
    if (!texture)
    {
        texture = m_registry.addComponent<OpenGLTextureComponent>(entity, std::make_shared<OpenGLTextureComponent>());
    }
}
