#ifndef ENGINE_OPENGL_SYSTEM_RENDERTRACKER_TEXTUREINDEX
#define ENGINE_OPENGL_SYSTEM_RENDERTRACKER_TEXTUREINDEX

#include <filesystem>
#include <list>
#include <map>

namespace Engine
{
class OpenGLTextureComponent;

namespace Systems
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

private:
    using textureData = std::pair<unsigned int, std::list<OpenGLTextureComponent *>>;
    using typeMap = std::map<unsigned int, textureData>;
    std::map<std::filesystem::path, typeMap> m_textures{};
};

} // namespace Systems

} // namespace Engine

#endif