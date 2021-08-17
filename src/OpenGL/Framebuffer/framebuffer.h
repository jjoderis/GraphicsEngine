#ifndef ENGINE_OPENGL_FRAMEBUFFER
#define ENGINE_OPENGL_FRAMEBUFFER

#include <vector>
#include <map>
#include <glad/glad.h>
#include "../../Core/Math/math.h"
#include <tuple>

namespace Engine {

// internal format, format, type, attachment
using ColorAttachment = std::tuple<unsigned int, unsigned int, unsigned int>;
using OtherAttachment = std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>;

class OpenGLFramebuffer {

public:

  OpenGLFramebuffer();
  OpenGLFramebuffer(const std::vector<ColorAttachment> &attachments);

  ~OpenGLFramebuffer();

  void bind();
  void unbind();
  unsigned int getTexture(int index = 0);
  void setClearColor(const Engine::Math::Vector4 &color, int textureIndex = 0);
  void setClearColorI(const Engine::Math::IVector4 &color, int textureIndex = 0);
  void clear();

  void getPixel(void* data, int x, int y, unsigned int format, unsigned int type, unsigned int attachment = GL_COLOR_ATTACHMENT0);

  void resize(int width, int height);

private:

  void buildFramebuffer();
  void eraseFramebuffer();
  void rebuildFramebuffer();

  unsigned int m_framebuffer{0};

  std::vector<ColorAttachment> m_colorAttachments{{GL_RGB, GL_RGB, GL_UNSIGNED_BYTE}};
  std::vector<unsigned int> m_drawAttachments{};
  std::vector<unsigned int> m_textures{};
  std::map<int, Engine::Math::Vector4> m_clearColors{};
  std::map<int, Engine::Math::IVector4> m_clearColorsI{};

  std::vector<OtherAttachment> m_otherAttachments{{GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, GL_DEPTH_STENCIL_ATTACHMENT}};
  std::vector<unsigned int> m_otherBuffers{};

  int m_width{800};
  int m_height{600};
};

}

#endif