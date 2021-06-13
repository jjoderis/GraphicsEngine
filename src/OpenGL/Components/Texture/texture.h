#ifndef ENGINE_OPENGL_COMPONENTS_TEXTURE
#define ENGINE_OPENGL_COMPONENTS_TEXTURE

#include <filesystem>
#include <vector>

namespace Engine
{
class OpenGLTextureComponent
{
public:
    // load texture from file at given path
    OpenGLTextureComponent();

    void addTexture(unsigned int buffer, unsigned int type);
    void editTexture(int index, unsigned int buffer, unsigned int type);
    using textureData = std::pair<unsigned int, unsigned int>;
    const std::vector<textureData> &getTextures() const;
    textureData &getTexture(int index);
    unsigned int getNumTextures() const;

    void bind();
    void unbind();

private:
    unsigned int m_numTextures{0};
    std::vector<textureData> m_textures;
};
} // namespace Engine

#endif