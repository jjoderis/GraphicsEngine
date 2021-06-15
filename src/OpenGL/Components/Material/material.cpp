#include "material.h"

#include <glad/glad.h>

Engine::OpenGLMaterialComponent::~OpenGLMaterialComponent()
{
    if (m_UBO != 0)
    {
        glDeleteBuffers(1, &m_UBO);
    }
}

void Engine::OpenGLMaterialComponent::setMaterialData(const ShaderMaterialData &materialData)
{
    if (materialData != m_dataInfo)
    {
        // TODO: keep data that is reused in new shader
        int dataSize{std::get<0>(materialData)};
        int oldDataSize(std::get<0>(m_dataInfo));

        std::vector<char> newData(dataSize, 0);

        if (m_UBO > 0 && dataSize != oldDataSize)
        {
            glDeleteBuffers(1, &m_UBO);
            m_UBO = 0;
        }

        newData.swap(m_data);
        m_dataInfo = materialData;
        update();
    }
}

void Engine::OpenGLMaterialComponent::update()
{
    int materialDataSize{std::get<0>(m_dataInfo)};

    if (m_UBO == 0)
    {
        glGenBuffers(1, &m_UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
        glBufferData(GL_UNIFORM_BUFFER, materialDataSize, NULL, GL_DYNAMIC_DRAW);
    }
    glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);

    glBufferSubData(GL_UNIFORM_BUFFER, 0, materialDataSize, m_data.data());
}

Engine::ShaderMaterialData &Engine::OpenGLMaterialComponent::getMaterialData() { return m_dataInfo; }

std::vector<char> &Engine::OpenGLMaterialComponent::getData() { return m_data; }

void Engine::OpenGLMaterialComponent::bind() { glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_UBO); }