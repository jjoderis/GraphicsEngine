#include "shaderTracker.h"

#include "../../../Core/ECS/registry.h"
#include "../../Components/Material/material.h"
#include "../../Components/Shader/shader.h"
#include "../../Components/Texture/texture.h"
#include <set>

void ensureMaterialAndTexture(unsigned int entity, Engine::Registry &registry);

Engine::Systems::OpenGLShaderTracker::OpenGLShaderTracker(Registry &registry) : m_registry{registry}
{
    m_addCallback = m_registry.onAdded<Engine::OpenGLShaderComponent>(
        [&](unsigned int addEntity, std::weak_ptr<OpenGLShaderComponent> shader)
        {
            ensureMaterialAndTexture(addEntity, m_registry);
            update(addEntity, shader.lock().get()); 
        });

    m_updateCallback = m_registry.onUpdate<Engine::OpenGLShaderComponent>(
        [&](unsigned int updateEntity, std::weak_ptr<OpenGLShaderComponent> shader)
        { update(updateEntity, shader.lock().get()); });

    m_swapCallback = m_registry.onComponentSwap<Engine::OpenGLShaderComponent>(
        [&](unsigned int swapEntity, std::weak_ptr<OpenGLShaderComponent> newShader)
        { update(swapEntity, newShader.lock().get()); });
}

void ensureMaterialAndTexture(unsigned int entity, Engine::Registry &registry) {
    if (!registry.hasComponent<Engine::OpenGLMaterialComponent>(entity)) {
        registry.createComponent<Engine::OpenGLMaterialComponent>(entity);
    }
    if (!registry.hasComponent<Engine::OpenGLTextureComponent>(entity)) {
        registry.createComponent<Engine::OpenGLTextureComponent>(entity);
    }
}

void Engine::Systems::OpenGLShaderTracker::update(unsigned int entity, OpenGLShaderComponent *shader)
{
    ShaderMaterialData matData{shader->getMaterialProperties()};

    // get the Owners of the ShaderComponent
    auto &owners{m_registry.getOwners<OpenGLShaderComponent>(entity)};

    // prevent multiple updates of same OpenMaterialComponents on different owners
    // TODO: make sure that the new material properties still fit all entities (more precisely their shaders)
    // need to change may already by an indicator for it not fitting if another owner has a ShaderComponent
    // since that one will most likely be responsible for the current material data
    std::set<OpenGLMaterialComponent *> uniqueMaterialComponents{};
    for (unsigned int owner : owners)
    {
        auto materialComponent = m_registry.getComponent<Engine::OpenGLMaterialComponent>(owner);

        uniqueMaterialComponents.emplace(materialComponent.get());
    }

    for (auto &materialComponent : uniqueMaterialComponents)
    {
        materialComponent->setMaterialData(matData);
        m_registry.updated<OpenGLMaterialComponent>(entity);
    }
}