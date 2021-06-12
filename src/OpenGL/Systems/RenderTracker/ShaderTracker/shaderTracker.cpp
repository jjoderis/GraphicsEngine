#include "shaderTracker.h"

#include "../../../../Core/Components/Render/render.h"
#include "../../../../Core/ECS/registry.h"
#include "../../../Components/Material/material.h"
#include "../../../Components/Shader/shader.h"
#include <glad/glad.h>

Engine::Systems::OpenGLRenderShaderTracker::OpenGLRenderShaderTracker(unsigned int entity, Registry &registry)
    : m_registry{registry}
{
    m_updateCallback = m_registry.onUpdate<Engine::OpenGLShaderComponent>(
        entity,
        [&](unsigned int updateEntity, std::weak_ptr<OpenGLShaderComponent> shader) { update(shader.lock().get()); });

    m_users.push_back(entity);

    update(registry.getComponent<OpenGLShaderComponent>(entity).get());
}

constexpr size_t matrixSize{16 * sizeof(float)};
constexpr size_t transformSize{2 * matrixSize};

void Engine::Systems::OpenGLRenderShaderTracker::update(OpenGLShaderComponent *shader)
{
    ShaderMaterialData matData{shader->getMaterialProperties()};

    for (unsigned int entity : m_users)
    {
        std::shared_ptr<OpenGLMaterialComponent> material = m_registry.getComponent<OpenGLMaterialComponent>(entity);
        // TODO: prevent changing the same material multiple times
        material->setMaterialData(matData);
        m_registry.updated<OpenGLMaterialComponent>(entity);
    }
}

const std::list<unsigned int> &Engine::Systems::OpenGLRenderShaderTracker::getUsers() const { return m_users; }