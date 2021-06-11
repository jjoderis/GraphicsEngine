#include "texture.h"

Engine::OpenGLTextureComponent::OpenGLTextureComponent(const fs::path& path) {
  int width, height, n;
  unsigned char *data = stbi_load(path.c_str(), &width, &height, &n, 0);

  if (data != NULL) {
    throw "Couldn't load texture";
  }

  GLenum type;

  switch(n) {
    case 3:
      type = GL_RGB;
      break;
    case 4:
      type = GL_RGBA;
      break;
    default:
      throw "Unknown texture type";
      break;
  }

  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, type, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  stbi_image_free(data);
}

Engine::OpenGLTextureComponent::~OpenGLTextureComponent() {
  glDeleteTextures(1, &m_texture);
}