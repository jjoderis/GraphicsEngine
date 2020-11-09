#include "geometry.h"

Engine::GeometryComponent::GeometryComponent() : m_vertices{}, m_faces{} {}
Engine::GeometryComponent::GeometryComponent(std::initializer_list<Math::Vector3> vertices, std::initializer_list<unsigned int> faces)
    : m_vertices{ vertices }, m_faces{ faces } {}

std::vector<Engine::Math::Vector3>& Engine::GeometryComponent::getVertices() {
    return m_vertices;
}

std::vector<unsigned int>& Engine::GeometryComponent::getFaces() {
    return m_faces;
}