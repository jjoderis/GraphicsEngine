#include "textureIndex.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace fs = std::filesystem;

Engine::Systems::OpenGLTextureIndex::~OpenGLTextureIndex()
{
    for (auto &typeData : m_textures)
    {
        for (auto &textureData : typeData.second)
        {
            glDeleteTextures(1, &textureData.first);
        }
    }
}

unsigned int
Engine::Systems::OpenGLTextureIndex::needTexture(const fs::path &path, unsigned int type, OpenGLTextureComponent *user)
{
    if (!m_textures.count(path) || !m_textures.at(path).count(type))
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
        default:
            throw "Unknown texture type";
            break;
        }

        unsigned int texture;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, colorType, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        if (!m_textures.count(path))
        {
            m_textures.emplace(path, typeMap{})
                .first->second.emplace(type, textureData{texture, std::list<OpenGLTextureComponent *>{user}});
        }
        else
        {
            typeMap &typeData{m_textures.at(path)};

            typeData.emplace(type, textureData{texture, std::list<OpenGLTextureComponent *>{user}});
        }

        return texture;
    }

    return m_textures.at(path).at(type).first;
}

void Engine::Systems::OpenGLTextureIndex::unneedTexture(const fs::path &path,
                                                        unsigned int type,
                                                        OpenGLTextureComponent *user)
{
    if (!m_textures.count(path) || !m_textures.at(path).count(type))
    {
        return;
    }

    typeMap &types{m_textures.at(path)};

    textureData &data{types.at(type)};

    std::list<OpenGLTextureComponent *> &users{data.second};
    users.remove(user);

    if (!users.size())
    {
        glDeleteTextures(1, &data.first);
        types.erase(type);

        if (!types.size())
        {
            m_textures.erase(path);
        }
    }
}