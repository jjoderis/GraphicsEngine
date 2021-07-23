#include "textureIndex.h"

#include "./textureLoader.h"
#include <glad/glad.h>

namespace fs = std::filesystem;

Engine::Util::OpenGLTextureHandler::OpenGLTextureHandler(unsigned int texture,
                                                         const TextureData &data,
                                                         OpenGLTextureIndex &index)
    : m_texture{texture}, m_data{data}, m_index{index}
{
    // increase number of users
    m_index.increaseUsers(m_data);
}

Engine::Util::OpenGLTextureHandler::OpenGLTextureHandler(const OpenGLTextureHandler &other)
    : m_texture{other.m_texture}, m_data{other.m_data}, m_index{other.m_index}
{
    // increase number of users
    m_index.increaseUsers(m_data);
}

Engine::Util::OpenGLTextureHandler &Engine::Util::OpenGLTextureHandler::operator=(const OpenGLTextureHandler &other)
{
    if (this == &other)
    {
        return *this;
    }

    m_index.decreaseUsers(m_data);
    m_index = other.m_index;
    m_data = other.m_data;
    m_index.increaseUsers(m_data);
    m_texture = other.m_texture;
}

Engine::Util::OpenGLTextureHandler::~OpenGLTextureHandler()
{
    // reduce the number of texture users by one
    m_index.decreaseUsers(m_data);
}

unsigned int Engine::Util::OpenGLTextureHandler::getTexture() const { return m_texture; }

std::filesystem::path Engine::Util::OpenGLTextureHandler::getPath() const { return std::get<0>(m_data); }
unsigned int Engine::Util::OpenGLTextureHandler::getType() const { return std::get<1>(m_data); }
unsigned int Engine::Util::OpenGLTextureHandler::pixelType() const { return std::get<2>(m_data); }
unsigned int Engine::Util::OpenGLTextureHandler::getMinFilter() const { return std::get<3>(m_data); }
unsigned int Engine::Util::OpenGLTextureHandler::getMagFilter() const { return std::get<4>(m_data); }
unsigned int Engine::Util::OpenGLTextureHandler::getWrapS() const { return std::get<5>(m_data); }
unsigned int Engine::Util::OpenGLTextureHandler::getWrapT() const { return std::get<6>(m_data); }

Engine::Util::OpenGLTextureIndex::~OpenGLTextureIndex()
{
    for (auto &textureData : m_textures)
    {
        glDeleteTextures(1, &textureData.second.first);
    }
}

Engine::Util::OpenGLTextureHandler Engine::Util::OpenGLTextureIndex::needTexture(const std::filesystem::path &path,
                                                                                 unsigned int type,
                                                                                 unsigned int pixelType,
                                                                                 unsigned int minFilter,
                                                                                 unsigned int magFilter,
                                                                                 unsigned int wrapS,
                                                                                 unsigned int wrapT)
{
    auto absPath = std::filesystem::canonical(path);
    TextureData data{absPath, type, pixelType, minFilter, magFilter, wrapS, wrapT};

    if (m_textures.find(data) != m_textures.end())
    {
        return OpenGLTextureHandler{m_textures.at(data).first, data, *this};
    }

    unsigned int texture = Util::loadTexture(path, type, pixelType, minFilter, magFilter, wrapS, wrapT).buffer;

    m_textures.emplace(data, TextureUsage{texture, 0});

    return OpenGLTextureHandler{texture, data, *this};
}

void Engine::Util::OpenGLTextureIndex::increaseUsers(const TextureData &data) { ++m_textures.at(data).second; }

void Engine::Util::OpenGLTextureIndex::decreaseUsers(const TextureData &data)
{
    auto &numUsers = m_textures.at(data).second;
    --numUsers;

    if (numUsers == 0)
    {
        glDeleteTextures(1, &m_textures.at(data).first);
        m_textures.erase(data);
    }
}