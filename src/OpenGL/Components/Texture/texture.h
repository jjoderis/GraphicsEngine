#ifndef ENGINE_OPENGL_COMPONENTS_TEXTURE
#define ENGINE_OPENGL_COMPONENTS_TEXTURE

#include <filesystem>
namespace Engine
{
class OpenGLTextureComponent
{
public:
    // load texture from file at given path
    OpenGLTextureComponent(const std::filesystem::path &path);
    ~OpenGLTextureComponent();

private:
    unsigned int m_texture{0};
};
} // namespace Engine

#endif