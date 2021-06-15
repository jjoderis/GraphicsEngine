#include "renderTracker.h"

#include "../../../Core/Components/Geometry/geometry.h"
#include "../../../Core/Components/Hierarchy/hierarchy.h"
#include "../../../Core/Components/Render/render.h"
#include "../../../Core/Components/Transform/transform.h"
#include "../../../Core/ECS/registry.h"
#include "../../Components/Material/material.h"
#include "../../Components/OpenGLGeometry/openGLGeometry.h"
#include "../../Components/Shader/shader.h"
#include "../../Components/Texture/texture.h"

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
    m_entities.emplace(entity, entityData{nullptr, nullptr, nullptr, nullptr, nullptr});

    ensureMaterial(entity);

    ensureGeometry(entity);

    ensureTransform(entity);

    // TODO: texture not necessary
    ensureTexture(entity);

    ensureShader(entity);

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

    std::get<0>(m_entities.at(entity)) = geometry.get();
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

    std::get<1>(m_entities.at(entity)) = material.get();
}

void Engine::Systems::OpenGLRenderTracker::ensureTransform(unsigned int entity)
{
    std::shared_ptr<TransformComponent> transform{m_registry.getComponent<TransformComponent>(entity)};
    if (!transform)
    {
        transform = m_registry.addComponent<TransformComponent>(entity, std::make_shared<TransformComponent>());
    }

    // check if the transform is not currently known
    if (m_transforms.find(transform.get()) == m_transforms.end())
    {
        m_transforms.try_emplace(transform.get(), entity, m_registry);
    }

    std::get<2>(m_entities.at(entity)) = transform.get();
}

void Engine::Systems::OpenGLRenderTracker::ensureShader(unsigned int entity)
{
    // we expect the shader to exist for now since creating the shader here would need some kind of base
    // shader code to be defined here
    std::shared_ptr<OpenGLShaderComponent> shader{m_registry.getComponent<OpenGLShaderComponent>(entity)};

    // check if the shader is not currently known
    if (m_shaders.find(shader.get()) == m_shaders.end())
    {
        m_shaders.try_emplace(shader.get(), entity, m_registry);
    }

    std::get<3>(m_entities.at(entity)) = shader.get();
}

void Engine::Systems::OpenGLRenderTracker::ensureTexture(unsigned int entity)
{
    std::shared_ptr<OpenGLTextureComponent> texture{m_registry.getComponent<OpenGLTextureComponent>(entity)};
    if (!texture)
    {
        texture = m_registry.addComponent<OpenGLTextureComponent>(entity, std::make_shared<OpenGLTextureComponent>());
    }

    std::get<4>(m_entities.at(entity)) = texture.get();
}

void Engine::Systems::OpenGLRenderTracker::render()
{
    auto renderableEntities = m_registry.getOwners<Engine::RenderComponent>();

    for (auto entities : renderableEntities)
    {
        for (auto entity : entities)
        {
            m_registry.getComponent<Engine::OpenGLShaderComponent>(entity)->useShader();
            m_registry.getComponent<Engine::OpenGLMaterialComponent>(entity)->bind();
            entityData &data{m_entities.at(entity)};
            unsigned int transformUBO{m_transforms.at(std::get<2>(data)).getBuffer()};
            glBindBufferBase(GL_UNIFORM_BUFFER, 1, transformUBO);

            std::get<4>(data)->bind();
            m_registry.getComponent<Engine::OpenGLGeometryComponent>(entity)->draw();
        }
    }

    glUseProgram(0);
}
