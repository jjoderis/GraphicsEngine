#include "textureLoader.h"

#include <glad/glad.h>
#include <stb_image.h>

Engine::Util::textureInfo Engine::Util::loadTexture(const std::filesystem::path &path,
                                                    unsigned int type,
                                                    unsigned int pixelType,
                                                    unsigned int minFilter,
                                                    unsigned int magFilter,
                                                    unsigned int wrapS,
                                                    unsigned int wrapT)
{
    int width, height, n;
    stbi_set_flip_vertically_on_load(true);
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
    glBindTexture(type, texture);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, wrapT);
    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, minFilter);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, magFilter);
    glTexImage2D(type, 0, pixelType, width, height, 0, colorType, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(type);

    stbi_image_free(data);

    return {width, height, texture};
}