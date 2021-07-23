#ifndef ENGINE_OPENGL_COMPONENTS_TEXTURE
#define ENGINE_OPENGL_COMPONENTS_TEXTURE

#include "../../Util/textureIndex.h"
#include <filesystem>
#include <vector>

namespace Engine
{
namespace Util
{
class OpenGLTextureHandler;
};

class OpenGLTextureComponent
{
public:
    // load texture from file at given path
    OpenGLTextureComponent();

    void addTexture(const Util::OpenGLTextureHandler &handler);
    void editTexture(int index, const Util::OpenGLTextureHandler &handler);
    const std::vector<Util::OpenGLTextureHandler> &getTextures() const;
    Util::OpenGLTextureHandler &getTexture(int index);
    unsigned int getNumTextures() const;

    void bind();
    void unbind();

private:
    unsigned int m_numTextures{0};
    std::vector<Util::OpenGLTextureHandler> m_textures;
};
} // namespace Engine

#endif