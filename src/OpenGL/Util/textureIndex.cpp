#include "textureIndex.h"

#include "./textureLoader.h"
#include <glad/glad.h>

namespace fs = std::filesystem;

Engine::Util::OpenGLTextureIndex::~OpenGLTextureIndex()
{
    for (auto &typeData : m_textures)
    {
        for (auto &textureData : typeData.second)
        {
            glDeleteTextures(1, &textureData.second.first);
        }
    }
}

unsigned int
Engine::Util::OpenGLTextureIndex::needTexture(const fs::path &path, unsigned int type, OpenGLTextureComponent *user)
{
    if (!m_textures.count(path) || !m_textures.at(path).count(type))
    {
        unsigned int texture = Util::loadTexture(path, type, GL_RGBA).buffer;
        m_buffers.emplace(texture, bufferData{path, type});

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

void Engine::Util::OpenGLTextureIndex::unneedTexture(const fs::path &path,
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
        m_buffers.erase(data.first);
        glDeleteTextures(1, &data.first);
        types.erase(type);

        if (!types.size())
        {
            m_textures.erase(path);
        }
    }
}

void Engine::Util::OpenGLTextureIndex::unneedTexture(unsigned int buffer, OpenGLTextureComponent *user)
{
    if (m_buffers.count(buffer))
    {
        auto data{m_buffers.at(buffer)};

        unneedTexture(data.first, data.second, user);
    }
}