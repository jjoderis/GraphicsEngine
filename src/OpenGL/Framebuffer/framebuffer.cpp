#include "framebuffer.h"

#include <cstddef>

void deleteFramebuffer(unsigned int &frameBuffer)
{
    if (frameBuffer > 0)
    {
        glDeleteFramebuffers(1, &frameBuffer);
    }
}

void deleteTexture(unsigned int &texture)
{
    if (texture > 0)
    {
        glDeleteTextures(1, &texture);
    }
}

void createFramebuffer(unsigned int &framebuffer)
{
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void createColorAttachment(
    unsigned int &texture, int width, int height, Engine::ColorAttachment &typeData, int attachmentOffset = 0)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(
        GL_TEXTURE_2D, 0, std::get<0>(typeData), width, height, 0, std::get<1>(typeData), std::get<2>(typeData), NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachmentOffset, GL_TEXTURE_2D, texture, 0);
}

void createOtherAttachment(unsigned int &texture, int width, int height, Engine::OtherAttachment &typeData)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(
        GL_TEXTURE_2D, 0, std::get<0>(typeData), width, height, 0, std::get<1>(typeData), std::get<2>(typeData), NULL);

    glFramebufferTexture2D(GL_FRAMEBUFFER, std::get<3>(typeData), GL_TEXTURE_2D, texture, 0);
}

Engine::OpenGLFramebuffer::OpenGLFramebuffer() { buildFramebuffer(); }

Engine::OpenGLFramebuffer::OpenGLFramebuffer(const std::vector<ColorAttachment> &attachments)
    : m_colorAttachments{attachments}
{
    buildFramebuffer();
}

void Engine::OpenGLFramebuffer::buildFramebuffer()
{
    createFramebuffer(m_framebuffer);

    m_textures.resize(m_colorAttachments.size(), 0);
    m_drawAttachments.resize(m_colorAttachments.size(), 0);

    for (int i = 0; i < m_colorAttachments.size(); ++i)
    {
        createColorAttachment(m_textures[i], m_width, m_height, m_colorAttachments[i], i);
        m_drawAttachments[i] = GL_COLOR_ATTACHMENT0 + i;
    }

    m_otherBuffers.resize(m_otherAttachments.size(), 0);

    for (int i = 0; i < m_otherAttachments.size(); ++i)
    {
        createOtherAttachment(m_otherBuffers[i], m_width, m_height, m_otherAttachments[i]);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        throw "Failed to create frame buffer";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Engine::OpenGLFramebuffer::eraseFramebuffer()
{
    deleteFramebuffer(m_framebuffer);

    for (auto &texture : m_textures)
    {
        deleteTexture(texture);
    }
    for (auto &otherBuffer : m_otherBuffers)
    {
        deleteTexture(otherBuffer);
    }
}

void Engine::OpenGLFramebuffer::rebuildFramebuffer()
{
    eraseFramebuffer();

    buildFramebuffer();
}

void Engine::OpenGLFramebuffer::resize(int width, int height)
{
    m_width = width;
    m_height = height;

    rebuildFramebuffer();
}

void Engine::OpenGLFramebuffer::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glViewport(0, 0, m_width, m_height);

    if (m_drawAttachments.size() > 1)
    {
        glDrawBuffers(m_drawAttachments.size(), m_drawAttachments.data());
    }
}

void Engine::OpenGLFramebuffer::unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

unsigned int Engine::OpenGLFramebuffer::getTexture(int index) { return m_textures.at(index); }

void Engine::OpenGLFramebuffer::setClearColor(const Engine::Vector4 &color, int textureIndex)
{
    if (textureIndex < m_textures.size())
    {
        m_clearColors.emplace(textureIndex, color);
    }
}

void Engine::OpenGLFramebuffer::setClearColorI(const Engine::IVector4 &color, int textureIndex)
{
    if (textureIndex < m_textures.size())
    {
        m_clearColorsI.emplace(textureIndex, color);
    }
}

void Engine::OpenGLFramebuffer::clear()
{
    bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int i = 0; i < m_textures.size(); ++i)
    {
        if (m_clearColors.find(i) != m_clearColors.end())
        {
            glClearBufferfv(GL_COLOR, i, m_clearColors.at(i).data());
        }
        if (m_clearColorsI.find(i) != m_clearColorsI.end())
        {
            glClearBufferiv(GL_COLOR, i, m_clearColorsI.at(i).data());
        }
    }

    unbind();
}

void Engine::OpenGLFramebuffer::getPixel(
    void *data, int x, int y, unsigned int format, unsigned int type, unsigned int attachment)
{
    bind();
    glReadBuffer(attachment);

    glReadPixels(x, y, 1, 1, format, type, data);

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    unbind();
}

Engine::OpenGLFramebuffer::~OpenGLFramebuffer() { eraseFramebuffer(); }