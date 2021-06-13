#include "textureIndex.h"

#include "../../../Util/textureLoader.h"
#include <glad/glad.h>

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
        unsigned int texture = Util::loadTexture(path, type).buffer;

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