#include "materialTracker.h"

#include "../../../../Core/ECS/registry.h"
#include "../../../Components/Material/material.h"
#include <glad/glad.h>

Engine::Systems::OpenGLRenderMaterialTracker::OpenGLRenderMaterialTracker(unsigned int entity, Registry &registry)
    : m_registry{registry}
{
    // this will track a material that is used by at least one entity with a RenderComponent
    m_updateCallback = m_registry.onUpdate<Engine::OpenGLMaterialComponent>(
        entity,
        [&](unsigned int updateEntity, std::weak_ptr<OpenGLMaterialComponent> material)
        { update(material.lock().get()); });

    update(registry.getComponent<OpenGLMaterialComponent>(entity).get());
}

Engine::Systems::OpenGLRenderMaterialTracker::~OpenGLRenderMaterialTracker()
{
    if (m_UBO != 0)
    {
        glDeleteBuffers(1, &m_UBO);
    }
}

void Engine::Systems::OpenGLRenderMaterialTracker::update(OpenGLMaterialComponent *material)
{
    int materialDataSize{std::get<0>(material->getMaterialData())};

    if (m_UBO > 0 && m_materialSize != materialDataSize)
    {
        glDeleteBuffers(1, &m_UBO);
        m_UBO = 0;
    }
    if (m_UBO == 0)
    {
        m_materialSize = materialDataSize;
        glGenBuffers(1, &m_UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
        glBufferData(GL_UNIFORM_BUFFER, materialDataSize, NULL, GL_DYNAMIC_DRAW);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, materialDataSize, material->getData().data());
}

unsigned int Engine::Systems::OpenGLRenderMaterialTracker::getBuffer() { return m_UBO; }