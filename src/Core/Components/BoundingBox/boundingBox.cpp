#include "boundingBox.h"

#include "../Geometry/geometry.h"
#include <limits>

Engine::BoundingBoxComponent::BoundingBoxComponent(const GeometryComponent &geometry)
{
    calculateBoundingBox(geometry);
}

void Engine::BoundingBoxComponent::calculateBoundingBox(const GeometryComponent &geometry)
{
    m_min = Math::Vector3{std::numeric_limits<float>::infinity(),
                          std::numeric_limits<float>::infinity(),
                          std::numeric_limits<float>::infinity()};
    m_max = Math::Vector3{-std::numeric_limits<float>::infinity(),
                          -std::numeric_limits<float>::infinity(),
                          -std::numeric_limits<float>::infinity()};

    for (auto &vertex : geometry.getVertices())
    {
        for (int i = 0; i < 3; ++i)
        {
            if (vertex.at(i) < m_min.at(i))
            {
                m_min.at(i) = vertex.at(i);
            }
            if (vertex.at(i) > m_max.at(i))
            {
                m_max.at(i) = vertex.at(i);
            }
        }
    }

    m_min -= 0.01;
    m_max += 0.01;
}

const Engine::Math::Vector3 &Engine::BoundingBoxComponent::getMin() const { return m_min; }
const Engine::Math::Vector3 &Engine::BoundingBoxComponent::getMax() const { return m_max; }