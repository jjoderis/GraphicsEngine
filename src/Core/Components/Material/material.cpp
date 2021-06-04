#include "material.h"

void Engine::MaterialComponent::setMaterialData(const ShaderMaterialData &materialData)
{
    if (materialData != m_dataInfo)
    {
        int dataSize{std::get<0>(materialData)};

        std::vector<char> newData(dataSize, 0);

        newData.swap(m_data);
        m_dataInfo = materialData;
    }
}

Engine::ShaderMaterialData &Engine::MaterialComponent::getMaterialData() { return m_dataInfo; }

std::vector<char> &Engine::MaterialComponent::getData() { return m_data; }