#include "renderer.h"

Engine::OpenGLRenderer::OpenGLRenderer(Registry& registry)
 : m_registry{registry}, m_cameraTracker{m_activeCameraUBO, registry}, 
   m_directionalLightsTracker{m_directionalLightsInfoUBO, registry},
   m_pointLightsTracker{m_pointLightsInfoUBO, registry},
   m_spotLightsTracker{m_spotLightsInfoUBO, registry}
{
}

Engine::OpenGLRenderer::~OpenGLRenderer() {
    glDeleteBuffers(1, &m_directionalLightsInfoUBO);
    glDeleteBuffers(1, &m_pointLightsInfoUBO);
    glDeleteBuffers(1, &m_spotLightsInfoUBO);
    glDeleteBuffers(1, &m_activeCameraUBO);
}

void Engine::OpenGLRenderer::render() {
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_activeCameraUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_directionalLightsInfoUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 4, m_pointLightsInfoUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_spotLightsInfoUBO);

    std::vector<std::shared_ptr<Engine::OpenGLRenderComponent>> renderComponents = m_registry.getComponents<Engine::OpenGLRenderComponent>();

    for( const std::shared_ptr<Engine::OpenGLRenderComponent>& renderComponent: renderComponents ) {
        renderComponent->render();
    }
}