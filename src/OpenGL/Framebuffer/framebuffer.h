#ifndef ENGINE_OPENGL_FRAMEBUFFER
#define ENGINE_OPENGL_FRAMEBUFFER

#include <vector>
#include <map>
#include <glad/glad.h>
#include "../../Core/Math/math.h"

namespace Engine {

class OpenGLFramebuffer {

public:

  OpenGLFramebuffer();
  OpenGLFramebuffer(const std::vector<unsigned int> &colorAttachments);

  ~OpenGLFramebuffer();

  void bind();
  void unbind();
  unsigned int getTexture(int index = 0);
  void setClearColor(const Engine::Math::Vector4 &color, int textureIndex = 0);
  void clear();

  void resize(int width, int height);

private:

  void buildFramebuffer();
  void rebuildFramebuffer();

  unsigned int m_framebuffer{0};

  std::vector<unsigned int> m_colorTypes{GL_RGB};
  std::vector<unsigned int> m_textures{0};
  std::vector<unsigned int> m_drawAttachments{};
  std::map<int, Engine::Math::Vector4> m_clearColors{};

  unsigned int m_depthStencilType{GL_DEPTH24_STENCIL8};
  unsigned int m_depthStencilBuffer{0};

  unsigned int m_depthType{0};
  unsigned int m_depthBuffer{0};

  unsigned int m_stencilType{0};
  unsigned int m_stencilBuffer{0};

  int m_width{800};
  int m_height{600};
};

}

#endif