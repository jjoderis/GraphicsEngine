#include "renderer.h"

Engine::OpenGLRenderer::OpenGLRenderer(Registry& registry)
 : m_registry{registry}, m_cameraTracker{m_activeCameraUBO, registry}, m_lightsTracker{m_lightsInfoUBO, registry}
{
}

Engine::OpenGLRenderer::~OpenGLRenderer() {
    glDeleteBuffers(1, &m_lightsInfoUBO);
    glDeleteBuffers(1, &m_activeCameraUBO);
}

void Engine::OpenGLRenderer::render() {
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_activeCameraUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_lightsInfoUBO);

    const std::vector<std::unique_ptr<Engine::OpenGLRenderComponent>> &renderComponents = m_registry.getComponents<Engine::OpenGLRenderComponent>();

    for( const std::unique_ptr<Engine::OpenGLRenderComponent>& renderComponent: renderComponents ) {
        renderComponent.get()->render();
    }
}