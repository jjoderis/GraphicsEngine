#ifndef ENGINE_OPENGL_TEXTURE_LOADER
#define ENGINE_OPENGL_TEXTURE_LOADER

#include <filesystem>
#include <glad/glad.h>

namespace Engine
{

namespace Util
{

struct textureInfo
{
    int width;
    int height;
    unsigned int buffer;
};

textureInfo loadTexture(const std::filesystem::path &path,
                        unsigned int type,
                        unsigned int pixelType = GL_RGB,
                        unsigned int minFilter = GL_LINEAR,
                        unsigned int magFilter = GL_LINEAR,
                        unsigned int wrapS = GL_REPEAT,
                        unsigned int wrapT = GL_REPEAT);

} // namespace Util

} // namespace Engine

#endif