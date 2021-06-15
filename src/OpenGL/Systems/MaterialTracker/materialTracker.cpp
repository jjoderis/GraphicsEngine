#include "materialTracker.h"

#include "../../../Core/ECS/registry.h"
#include "../../Components/Material/material.h"
#include "../../Components/Shader/shader.h"
#include <set>

Engine::Systems::OpenGLMaterialTracker::OpenGLMaterialTracker(Registry &registry) : m_registry{registry}
{
    m_addCallback = m_registry.onAdded<Engine::OpenGLMaterialComponent>(
        [&](unsigned int addEntity, std::weak_ptr<OpenGLMaterialComponent> material)
        { update(addEntity, material.lock().get()); });

    m_swapCallback = m_registry.onComponentSwap<Engine::OpenGLMaterialComponent>(
        [&](unsigned int swapEntity, std::weak_ptr<OpenGLMaterialComponent> newMaterial)
        { update(swapEntity, newMaterial.lock().get()); });
}

void Engine::Systems::OpenGLMaterialTracker::update(unsigned int entity, OpenGLMaterialComponent *material)
{

    if (auto shader = m_registry.getComponent<Engine::OpenGLShaderComponent>(entity))
    {
        ShaderMaterialData matData{shader->getMaterialProperties()};

        // TODO: make sure that changing this here won't break rendering for another entity that uses this component
        // see ShaderTracker
        material->setMaterialData(matData);
    }
}