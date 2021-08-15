#ifndef ENGINE_OPENGL_UTIL_TEXTUREINDEX
#define ENGINE_OPENGL_UTIL_TEXTUREINDEX

#include <filesystem>
#include <glad/glad.h>
#include <list>
#include <map>

namespace Engine
{
class OpenGLTextureComponent;

namespace Util
{
class OpenGLTextureIndex;
// path to texture file, OpenGL type for texture, OpenGL type for pixel colors, minFilter, magFilter, wrapS, wrapT
using TextureData = std::
    tuple<std::filesystem::path, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int>;

class OpenGLTextureHandler
{
public:
    OpenGLTextureHandler(unsigned int texture, const TextureData &data, OpenGLTextureIndex &index);
    OpenGLTextureHandler(const OpenGLTextureHandler &other);
    ~OpenGLTextureHandler();
    OpenGLTextureHandler &operator=(const OpenGLTextureHandler &other);

    unsigned int getTexture() const;

    std::filesystem::path getPath() const;
    unsigned int getType() const;
    unsigned int pixelType() const;
    unsigned int getMinFilter() const;
    unsigned int getMagFilter() const;
    unsigned int getWrapS() const;
    unsigned int getWrapT() const;

private:
    unsigned int m_texture;

    OpenGLTextureIndex &m_index;
    TextureData m_data;
};

class OpenGLTextureIndex
{
public:
    OpenGLTextureIndex() {}
    OpenGLTextureIndex(const OpenGLTextureIndex &other) = delete;

    ~OpenGLTextureIndex();

    Engine::Util::OpenGLTextureHandler needTexture(const std::filesystem::path &path,
                                                   unsigned int type = GL_TEXTURE_2D,
                                                   unsigned int pixelType = GL_RGB,
                                                   unsigned int minFilter = GL_LINEAR,
                                                   unsigned int magFilter = GL_LINEAR,
                                                   unsigned int wrapS = GL_REPEAT,
                                                   unsigned int wrapT = GL_REPEAT);

private:
    friend class OpenGLTextureHandler;

    // texture, #users
    using TextureUsage = std::pair<unsigned int, unsigned int>;
    std::map<TextureData, TextureUsage> m_textures{};

    void increaseUsers(const TextureData &data);

    void decreaseUsers(const TextureData &data);
};

} // namespace Util

} // namespace Engine

#endif