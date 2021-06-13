#include "textureLoader.h"

#include <glad/glad.h>
#include <stb_image.h>

Engine::Util::textureInfo
Engine::Util::loadTexture(const std::filesystem::path &path, unsigned int type, unsigned int pixelType)
{
    // TODO: create texture with given type
    int width, height, n;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &n, 0);

    if (data == NULL)
    {
        throw "Couldn't load texture";
    }

    GLenum colorType;

    switch (n)
    {
    case 3:
        colorType = GL_RGB;
        break;
    case 4:
        colorType = GL_RGBA;
        break;
    case 2:
        colorType = GL_RG;
        break;
    case 1:
        colorType = GL_RED;
    default:
        throw "Unknown texture type";
        break;
    }

    unsigned int texture;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, pixelType, width, height, 0, colorType, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    return {width, height, texture};
}