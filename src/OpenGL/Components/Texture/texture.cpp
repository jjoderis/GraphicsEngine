#include "texture.h"

namespace fs = std::filesystem;

Engine::OpenGLTextureComponent::OpenGLTextureComponent() {}

void Engine::OpenGLTextureComponent::addTexture(const fs::path &path, unsigned int type)
{
    ++m_numTextures;
    m_textures.emplace_back(textureData{path, type});
}

void Engine::OpenGLTextureComponent::editTexture(int index, const fs::path &path, unsigned int type)
{
    m_textures[index].first = path;
    m_textures[index].second = type;
}

const std::vector<Engine::OpenGLTextureComponent::textureData> &Engine::OpenGLTextureComponent::getTextures() const
{
    return m_textures;
};

unsigned int Engine::OpenGLTextureComponent::getNumTextures() const { return m_numTextures; }