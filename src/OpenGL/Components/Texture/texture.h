#ifndef ENGINE_OPENGL_COMPONENTS_TEXTURE
#define ENGINE_OPENGL_COMPONENTS_TEXTURE

#include "../../../Util/fileHandling.h"
#include <glad/glad.h>
#include <stb_image.h>

namespace Engine {
  class OpenGLTextureComponent {
  public:
    // load texture from file at given path
    OpenGLTextureComponent(const fs::path& path);
    ~OpenGLTextureComponent();

  private:
    unsigned int m_texture{ 0 };
  };
}

#endif