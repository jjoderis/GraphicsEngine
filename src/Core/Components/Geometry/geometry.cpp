#include "geometry.h"

Engine::GeometryComponent::GeometryComponent() : m_vertices{} {}
Engine::GeometryComponent::GeometryComponent(std::initializer_list<Math::Vector3> vertices) : m_vertices{ vertices } {}

std::vector<Engine::Math::Vector3>& Engine::GeometryComponent::getVertices() {
    return m_vertices;
}