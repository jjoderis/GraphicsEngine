#ifndef ENGINE_OPENGL_UTIL_TEXTUREINDEX
#define ENGINE_OPENGL_UTIL_TEXTUREINDEX

#include <filesystem>
#include <list>
#include <map>

namespace Engine
{
class OpenGLTextureComponent;

namespace Util
{

class OpenGLTextureIndex
{
public:
    OpenGLTextureIndex() {}
    OpenGLTextureIndex(const OpenGLTextureIndex &other) = delete;

    ~OpenGLTextureIndex();

    unsigned int needTexture(const std::filesystem::path &path, unsigned int type, OpenGLTextureComponent *user);
    // TODO: name?
    void unneedTexture(const std::filesystem::path &path, unsigned int type, OpenGLTextureComponent *user);

    void unneedTexture(unsigned int buffer, OpenGLTextureComponent *user);

private:
    using textureData = std::pair<unsigned int, std::list<OpenGLTextureComponent *>>;
    using typeMap = std::map<unsigned int, textureData>;
    std::map<std::filesystem::path, typeMap> m_textures{};

    using bufferData = std::pair<std::filesystem::path, unsigned int>;
    std::map<unsigned int, bufferData> m_buffers;
};

} // namespace Util

} // namespace Engine

#endif