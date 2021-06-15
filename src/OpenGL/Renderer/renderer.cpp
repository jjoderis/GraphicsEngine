#include "renderer.h"

#include "../../Core/ECS/registry.h"
#include "../Components/Material/material.h"
#include "../Components/OpenGLGeometry/openGLGeometry.h"
#include "../Components/OpenGLTransform/openGLTransform.h"
#include "../Components/Shader/shader.h"
#include "../Components/Texture/texture.h"

Engine::OpenGLRenderer::OpenGLRenderer(Registry &registry)
    : m_registry{registry}, m_cameraTracker{m_activeCameraUBO, registry}, m_ambientLightsTracker{m_ambientLightsInfoUBO,
                                                                                                 registry},
      m_directionalLightsTracker{m_directionalLightsInfoUBO, registry}, m_pointLightsTracker{m_pointLightsInfoUBO,
                                                                                             registry},
      m_spotLightsTracker{m_spotLightsInfoUBO, registry}, m_renderTracker{registry}
{
}

Engine::OpenGLRenderer::~OpenGLRenderer()
{
    glDeleteBuffers(1, &m_ambientLightsInfoUBO);
    glDeleteBuffers(1, &m_directionalLightsInfoUBO);
    glDeleteBuffers(1, &m_pointLightsInfoUBO);
    glDeleteBuffers(1, &m_spotLightsInfoUBO);
    glDeleteBuffers(1, &m_activeCameraUBO);
}

void Engine::OpenGLRenderer::render()
{
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_activeCameraUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_ambientLightsInfoUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 4, m_directionalLightsInfoUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_pointLightsInfoUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_spotLightsInfoUBO);

    auto renderableEntities = m_registry.getOwners<Engine::RenderComponent>();

    for (auto entities : renderableEntities)
    {
        for (auto entity : entities)
        {
            m_registry.getComponent<Engine::OpenGLShaderComponent>(entity)->useShader();
            m_registry.getComponent<Engine::OpenGLMaterialComponent>(entity)->bind();

            m_registry.getComponent<Engine::OpenGLTransformComponent>(entity)->bind();

            auto texture = m_registry.getComponent<Engine::OpenGLTextureComponent>(entity);
            texture->bind();
            m_registry.getComponent<Engine::OpenGLGeometryComponent>(entity)->draw();
            texture->unbind();
        }
    }

    glUseProgram(0);
}