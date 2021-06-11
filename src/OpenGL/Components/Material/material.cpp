#include "material.h"

void Engine::OpenGLMaterialComponent::setMaterialData(const ShaderMaterialData &materialData)
{
    if (materialData != m_dataInfo)
    {
        int dataSize{std::get<0>(materialData)};

        std::vector<char> newData(dataSize, 0);

        newData.swap(m_data);
        m_dataInfo = materialData;
    }
}

Engine::ShaderMaterialData &Engine::OpenGLMaterialComponent::getMaterialData() { return m_dataInfo; }

std::vector<char> &Engine::OpenGLMaterialComponent::getData() { return m_data; }