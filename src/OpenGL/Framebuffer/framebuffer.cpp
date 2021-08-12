#include "framebuffer.h"

#include <cstddef>

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

unsigned int getFormat(unsigned int internalFormat) {
  switch(internalFormat) {
    case GL_DEPTH24_STENCIL8:
      return GL_DEPTH_STENCIL;
    default:
      return internalFormat; 
  }
}

bool isColor(unsigned int format) {
  return (format == GL_RGB) || (format == GL_RGBA);
}

unsigned int getAttachment(unsigned int format) {
  if (isColor(format)) {
    return GL_COLOR_ATTACHMENT0;
  }

  switch(format) {
    case GL_DEPTH_STENCIL:
      return GL_DEPTH_STENCIL_ATTACHMENT;
    case GL_DEPTH:
      return GL_DEPTH_ATTACHMENT;
    case GL_STENCIL:
      return GL_STENCIL_ATTACHMENT;
  }
};

unsigned int getType(unsigned int internalFormat) {
  switch(internalFormat) {
    case GL_DEPTH24_STENCIL8:
      return GL_UNSIGNED_INT_24_8;
    default:
      return GL_UNSIGNED_BYTE;
  }
}

void createFramebuffer(unsigned int &framebuffer) {
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void createAttachment(unsigned int &texture, unsigned int internalFormat, int width, int height, int attachmentOffset = 0) {
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    auto format{getFormat(internalFormat)};
    
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, getType(internalFormat), NULL);

    if (isColor) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, getAttachment(format) + attachmentOffset, GL_TEXTURE_2D, texture, 0);
}

Engine::OpenGLFramebuffer::OpenGLFramebuffer() {
  buildFramebuffer();
}

Engine::OpenGLFramebuffer::OpenGLFramebuffer(const std::vector<unsigned int> &colorAttachmentType)
  : m_colorTypes{colorAttachmentType}
{
  buildFramebuffer();
}

void Engine::OpenGLFramebuffer::buildFramebuffer() {
  createFramebuffer(m_framebuffer);

  m_textures.resize(m_colorTypes.size(), 0);
  m_drawAttachments.resize(m_colorTypes.size(), 0);

  for (int i = 0; i < m_colorTypes.size(); ++i) {
    createAttachment(m_textures.at(i), m_colorTypes.at(i), m_width, m_height, i);
    m_drawAttachments[i] = GL_COLOR_ATTACHMENT0+i;
  }

  if (m_depthStencilType) {
    createAttachment(m_depthStencilBuffer, m_depthStencilType, m_width, m_height);
  }

  if (m_depthType) {
    createAttachment(m_depthBuffer, m_depthType, m_width, m_height);
  }

  if (m_stencilType) {
    createAttachment(m_stencilBuffer, m_stencilType, m_width, m_height);
  }

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    throw "Failed to create frame buffer";
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Engine::OpenGLFramebuffer::rebuildFramebuffer() {
  deleteFramebuffer(m_framebuffer);

  for (auto &texture : m_textures) {
    deleteTexture(texture);
  }
  deleteTexture(m_depthStencilBuffer);
  deleteTexture(m_depthBuffer);
  deleteTexture(m_stencilBuffer);

  buildFramebuffer();
}

void Engine::OpenGLFramebuffer::resize(int width, int height) {
    m_width = width;
    m_height = height;

    rebuildFramebuffer();
}

void Engine::OpenGLFramebuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glViewport(0, 0, m_width, m_height);
}

void Engine::OpenGLFramebuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

unsigned int Engine::OpenGLFramebuffer::getTexture(int index) {
    return m_textures.at(index);
}

void Engine::OpenGLFramebuffer::setClearColor(const Engine::Math::Vector4 &color, int textureIndex) {
  m_clearColors.emplace(textureIndex, color);
}

void Engine::OpenGLFramebuffer::clear(){
    bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (int i = 0; i < m_textures.size(); ++i) {
      if (m_clearColors.find(i) != m_clearColors.end()) {
        glDrawBuffer(GL_COLOR_ATTACHMENT0+i);
        auto &color{m_clearColors.at(i)};
        glClearColor(color(0), color(1), color(2), color(3));
        glClear(GL_COLOR_BUFFER_BIT);
      } 
    }

    if (m_drawAttachments.size() > 1) {
      glDrawBuffers(m_drawAttachments.size(), m_drawAttachments.data());
    }

    unbind();
}

Engine::OpenGLFramebuffer::~OpenGLFramebuffer() {
    deleteFramebuffer(m_framebuffer);

    for (auto &texture : m_textures) {
      deleteTexture(texture);
    }

    deleteTexture(m_depthStencilBuffer);
    deleteTexture(m_depthBuffer);
    deleteTexture(m_stencilBuffer);
}