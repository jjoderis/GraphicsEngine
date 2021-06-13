#include "textureTracker.h"

#include "../../../../Core/ECS/registry.h"
#include "../../../Components/Texture/texture.h"
#include <glad/glad.h>

namespace fs = std::filesystem;

Engine::Systems::OpenGLRenderTextureTracker::OpenGLRenderTextureTracker(unsigned int entity,
                                                                        Registry &registry,
                                                                        std::shared_ptr<OpenGLTextureIndex> index)
    : m_registry{registry}, m_index{index}
{
    m_updateCallback = m_registry.onUpdate<OpenGLTextureComponent>(
        entity,
        [&](unsigned int updateEntity, std::weak_ptr<OpenGLTextureComponent> texture)
        { update(texture.lock().get()); });

    update(registry.getComponent<OpenGLTextureComponent>(entity).get());
}

Engine::Systems::OpenGLRenderTextureTracker::~OpenGLRenderTextureTracker()
{
    // TODO: Unbind texture data from index
}

void Engine::Systems::OpenGLRenderTextureTracker::update(OpenGLTextureComponent *texture)
{
    if (m_numTextures != texture->getNumTextures())
    {
        m_numTextures = texture->getNumTextures();
        m_textures.resize(m_numTextures, textureData{fs::path{}, 0, 0});
    }

    auto &newDataVector{texture->getTextures()};

    for (int i = 0; i < m_numTextures; ++i)
    {
        textureData &currData{m_textures[i]};
        auto &newData{newDataVector[i]};

        // check if the path or type changed
        if (std::get<0>(currData) != newData.first || std::get<1>(currData) != newData.second)
        {
            // TODO: check for old data and remove if necessary
            unsigned int textureIndex = m_index->needTexture(newData.first, newData.second, texture);
            std::get<0>(currData) = newData.first;
            std::get<1>(currData) = newData.second;
            std::get<2>(currData) = textureIndex;
        }
    }
}

void Engine::Systems::OpenGLRenderTextureTracker::bindTextures()
{
    for (int i = 0; i < m_numTextures; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        // TODO: bind actual texture type
        glBindTexture(GL_TEXTURE_2D, std::get<2>(m_textures[0]));
    }
}

void Engine::Systems::OpenGLRenderTextureTracker::unbindTextures() {}