#ifndef ENGINE_OPENGL_COMPONENTS_TEXTURE
#define ENGINE_OPENGL_COMPONENTS_TEXTURE

#include <filesystem>
#include <string>
#include <vector>

namespace Engine
{
class OpenGLTextureComponent
{
public:
    // load texture from file at given path
    OpenGLTextureComponent();

    void addTexture(const std::filesystem::path &path, unsigned int type);
    using textureData = std::pair<std::filesystem::path, unsigned int>;
    const std::vector<textureData> &getTextures() const;
    unsigned int getNumTextures() const;

private:
    unsigned int m_numTextures{0};
    std::vector<textureData> m_textures;
};
} // namespace Engine

#endif