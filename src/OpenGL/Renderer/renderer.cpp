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
      m_spotLightsTracker{m_spotLightsInfoUBO, registry}
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

void Engine::OpenGLRenderer::render(const std::vector<unsigned int> &renderables)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_activeCameraUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_ambientLightsInfoUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 4, m_directionalLightsInfoUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 5, m_pointLightsInfoUBO);
    glBindBufferBase(GL_UNIFORM_BUFFER, 6, m_spotLightsInfoUBO);

    for (auto entity : renderables)
    {
        m_registry.getComponent<Engine::OpenGLShaderComponent>(entity)->useShader();
        m_registry.getComponent<Engine::OpenGLMaterialComponent>(entity)->bind();

        m_registry.getComponent<Engine::OpenGLTransformComponent>(entity)->bind();

        auto texture = m_registry.getComponent<Engine::OpenGLTextureComponent>(entity);
        texture->bind();
        m_registry.getComponent<Engine::OpenGLGeometryComponent>(entity)->draw();
        texture->unbind();
    }

    glUseProgram(0);
}

void deleteFramebuffer(unsigned int &frameBuffer) {
    if (frameBuffer > 0) {
        glDeleteFramebuffers(1, &frameBuffer);
    }
}

void deleteTexture(unsigned int &texture) {
    if (texture > 0) {
        glDeleteTextures(1, &texture);
    }
}

void createFramebuffer(unsigned int &framebuffer, unsigned int &texture, unsigned int &depthStencil, int width, int height) {
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    glGenTextures(1, &depthStencil);
    glBindTexture(GL_TEXTURE_2D, depthStencil);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencil, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw "Failed to create frame buffer";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Engine::OpenGLFramebuffer::OpenGLFramebuffer() {
    createFramebuffer(m_framebuffer, m_texture, m_depthStencil, m_width, m_height);
}

void Engine::OpenGLFramebuffer::resize(int width, int height) {
    deleteFramebuffer(m_framebuffer);
    deleteTexture(m_texture);
    deleteTexture(m_depthStencil);

    m_width = width;
    m_height = height;

    createFramebuffer(m_framebuffer, m_texture, m_depthStencil, m_width, m_height);
}

void Engine::OpenGLFramebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glViewport(0, 0, m_width, m_height);
}

void Engine::OpenGLFramebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Engine::OpenGLFramebuffer::getTexture() {
    return m_texture;
}

void Engine::OpenGLFramebuffer::clear(){
    bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    unbind();
}

Engine::OpenGLFramebuffer::~OpenGLFramebuffer() {
    deleteFramebuffer(m_framebuffer);
    deleteTexture(m_texture);
    deleteTexture(m_depthStencil);
}