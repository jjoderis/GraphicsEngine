#include "material.h"

Engine::MaterialComponent::MaterialComponent() : m_diffuseColor{1.0, 1.0, 1.0, 1.0}, m_specularColor{1.0, 1.0, 1.0, 1.0}
{
}
Engine::MaterialComponent::MaterialComponent(float r, float g, float b, float a)
    : m_diffuseColor{r, g, b, a}, m_specularColor{r, g, b, a}
{
}
Engine::MaterialComponent::MaterialComponent(Math::Vector4 &color) : m_diffuseColor{color}, m_specularColor{color} {}

Engine::Math::Vector4 &Engine::MaterialComponent::getDiffuseColor() { return m_diffuseColor; }
void Engine::MaterialComponent::setDiffuseColor(const Math::Vector4 &color) { m_diffuseColor = color; }

Engine::Math::Vector4 &Engine::MaterialComponent::getSpecularColor() { return m_specularColor; }
void Engine::MaterialComponent::setSpecularColor(const Math::Vector4 &color) { m_specularColor = color; }

float Engine::MaterialComponent::getSpecularExponent() const { return m_specularExponent; }
float &Engine::MaterialComponent::getSpecularExponent() { return m_specularExponent; }
void Engine::MaterialComponent::setSpecularExponent(float exp) { m_specularExponent = exp; }