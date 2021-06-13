#include "texture.h"

#include <glad/glad.h>

namespace fs = std::filesystem;

Engine::OpenGLTextureComponent::OpenGLTextureComponent() {}

void Engine::OpenGLTextureComponent::addTexture(unsigned int buffer, unsigned int type)
{
    ++m_numTextures;
    m_textures.emplace_back(textureData{buffer, type});
}

void Engine::OpenGLTextureComponent::editTexture(int index, unsigned int buffer, unsigned int type)
{
    m_textures[index].first = buffer;
    m_textures[index].second = type;
}

const std::vector<Engine::OpenGLTextureComponent::textureData> &Engine::OpenGLTextureComponent::getTextures() const
{
    return m_textures;
};
Engine::OpenGLTextureComponent::textureData &Engine::OpenGLTextureComponent::getTexture(int index)
{
    return m_textures[index];
}

unsigned int Engine::OpenGLTextureComponent::getNumTextures() const { return m_numTextures; }

void Engine::OpenGLTextureComponent::bind()
{
    for (int i = 0; i < m_numTextures; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        // TODO: bind actual texture type
        glBindTexture(GL_TEXTURE_2D, m_textures[i].first);
    }
}

void Engine::OpenGLTextureComponent::unbind()
{
    for (int i = 0; i < m_numTextures; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}