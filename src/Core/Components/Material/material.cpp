#include "material.h"

Engine::MaterialComponent::MaterialComponent() : m_color{1.0, 1.0, 1.0, 1.0} {}
Engine::MaterialComponent::MaterialComponent(float r, float g, float b, float a) : m_color{r, g, b, a} {}
Engine::MaterialComponent::MaterialComponent(Math::Vector4 &color) : m_color{color} {}

Engine::Math::Vector4 &Engine::MaterialComponent::getColor() { return m_color; }