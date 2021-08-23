#include "light.h"

Engine::Light::Light() {}

Engine::Light::Light(const Vector3 &color) : m_color{color} {}

void Engine::Light::setColor(const Vector3 &color) { m_color = color; }

Engine::Vector3 &Engine::Light::getColor() { return m_color; }

Engine::DirectionalLightComponent::DirectionalLightComponent() {}
Engine::DirectionalLightComponent::DirectionalLightComponent(const Vector3 &color, const Vector3 &direction)
    : Light(color), m_direction{direction}
{
}

void Engine::DirectionalLightComponent::setDirection(const Vector3 &direction) { m_direction = direction; }

Engine::Vector3 &Engine::DirectionalLightComponent::getDirection() { return m_direction; }

Engine::PunctualLight::PunctualLight() {}
Engine::PunctualLight::PunctualLight(const Vector3 &color, const Vector3 &position, float intensity)
    : Light(color), m_position{position}, m_intensity{intensity}
{
}

void Engine::PunctualLight::setPosition(const Vector3 &position) { m_position = position; }

Engine::Vector3 &Engine::PunctualLight::getPosition() { return m_position; }

void Engine::PunctualLight::setIntensity(float intensity) { m_intensity = intensity; }

float Engine::PunctualLight::getIntensity() { return m_intensity; }

Engine::PointLightComponent::PointLightComponent() {}
Engine::PointLightComponent::PointLightComponent(const Vector3 &color, const Vector3 &position, float intensity)
    : PunctualLight(color, position, intensity)
{
}

Engine::SpotLightComponent::SpotLightComponent() {}
Engine::SpotLightComponent::SpotLightComponent(
    const Vector3 &color, const Vector3 &position, float intensity, float cutoff, float penumbra)
    : PunctualLight(color, position, intensity), m_cutoffAngle{cutoff}, m_penumbraAngle{penumbra}
{
}

void Engine::SpotLightComponent::setCutoff(float angle) { m_cutoffAngle = angle; }

float Engine::SpotLightComponent::getCutoff() { return m_cutoffAngle; }

void Engine::SpotLightComponent::setPenumbra(float angle) { m_penumbraAngle = angle; }

float Engine::SpotLightComponent::getPenumbra() { return m_penumbraAngle; }