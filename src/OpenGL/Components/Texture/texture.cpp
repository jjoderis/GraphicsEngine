#include "texture.h"

#include "../../Util/textureIndex.h"
#include <glad/glad.h>

namespace fs = std::filesystem;

Engine::OpenGLTextureComponent::OpenGLTextureComponent() {}

void Engine::OpenGLTextureComponent::addTexture(const Util::OpenGLTextureHandler &handler)
{
    ++m_numTextures;
    m_textures.emplace_back(handler);
}

void Engine::OpenGLTextureComponent::editTexture(int index, const Util::OpenGLTextureHandler &handler)
{
    m_textures[index] = handler;
}

const std::vector<Engine::Util::OpenGLTextureHandler> &Engine::OpenGLTextureComponent::getTextures() const
{
    return m_textures;
};

Engine::Util::OpenGLTextureHandler &Engine::OpenGLTextureComponent::getTexture(int index) { return m_textures[index]; }

unsigned int Engine::OpenGLTextureComponent::getNumTextures() const { return m_numTextures; }

void Engine::OpenGLTextureComponent::bind()
{
    for (int i = 0; i < m_numTextures; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        // TODO: bind actual texture type
        glBindTexture(GL_TEXTURE_2D, m_textures[i].getTexture());
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