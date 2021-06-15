#include "material.h"

#include <algorithm>
#include <cstring>
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
        int dataSize{materialData.first};
        int oldDataSize(m_dataInfo.first);

        std::vector<char> newData(dataSize, 0);

        auto oldUniformsData{m_dataInfo.second};

        for (auto newUniformData : materialData.second)
        {
            auto itr = std::find_if(oldUniformsData.begin(),
                                    oldUniformsData.end(),
                                    [&](MaterialUniformData &entry) {
                                        return std::get<0>(entry) == std::get<0>(newUniformData) &&
                                               std::get<1>(entry) == std::get<1>(newUniformData);
                                    });
            if (itr != oldUniformsData.end())
            {
                switch (std::get<1>(*itr.base()))
                {
                case GL_FLOAT:
                    std::memcpy(newData.data() + std::get<2>(newUniformData),
                                m_data.data() + std::get<2>(*itr.base()),
                                sizeof(float));
                    break;

                case GL_FLOAT_VEC4:
                    std::memcpy(newData.data() + std::get<2>(newUniformData),
                                m_data.data() + std::get<2>(*itr.base()),
                                4 * sizeof(float));
                    break;
                case GL_FLOAT_VEC3:
                    std::memcpy(newData.data() + std::get<2>(newUniformData),
                                m_data.data() + std::get<2>(*itr.base()),
                                3 * sizeof(float));
                    break;
                }
            }
        }

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
    int materialDataSize{m_dataInfo.first};

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