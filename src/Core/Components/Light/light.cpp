#include "light.h"

Engine::Light::Light() {}

void Engine::Light::setColor(const Math::Vector3& color) {
    m_color = color;
}

Engine::Math::Vector3& Engine::Light::getColor() {
    return m_color;
}

Engine::DirectionalLightComponent::DirectionalLightComponent() {}

void Engine::DirectionalLightComponent::setDirection(const Math::Vector3& direction) {
    m_direction = direction;
}

Engine::Math::Vector3& Engine::DirectionalLightComponent::getDirection() {
    return m_direction;
}


Engine::PunctualLight::PunctualLight() {}

void Engine::PunctualLight::setPosition(const Math::Vector3& position) {
    m_position = position;
}

Engine::Math::Vector3& Engine::PunctualLight::getPosition() {
    return m_position;
}

void Engine::PunctualLight::setIntensity(float intensity) {
    m_intensity = intensity;
}

float Engine::PunctualLight::getIntensity() {
    return m_intensity;
}

Engine::PointLightComponent::PointLightComponent() {}

Engine::SpotLightComponent::SpotLightComponent() {}

void Engine::SpotLightComponent::setCutoff(float angle) {
    m_cutoffAngle = angle;
}

float Engine::SpotLightComponent::getCutoff() {
    return m_cutoffAngle;
}

void Engine::SpotLightComponent::setPenumbra(float angle) {
    m_penumbraAngle = angle;
}

float Engine::SpotLightComponent::getPenumbra() {
    return m_penumbraAngle;
}