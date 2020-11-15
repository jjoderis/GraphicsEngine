#include "transform.h"


Engine::TransformComponent::TransformComponent() {
    m_modelMatrix.setIdentity();
    m_modelMatrixInverse.setIdentity();
}

void Engine::TransformComponent::translate(Math::Vector3& translation) {
    m_translation += translation;
}
void Engine::TransformComponent::setTranslation(Math::Vector3& translation) {
    m_translation = translation;
}
Engine::Math::Vector3& Engine::TransformComponent::getTranslation(){
    return m_translation;
}

void Engine::TransformComponent::scale(Math::Vector3& scaling) {
    m_scaling *= scaling;
}
void Engine::TransformComponent::setScale(Math::Vector3& scaling) {
    m_scaling = scaling;
}
Engine::Math::Vector3& Engine::TransformComponent::getScaling() {
    return m_scaling;
}

void Engine::TransformComponent::rotate(Math::Vector3& rotation){
    m_rotation += rotation;
}
void Engine::TransformComponent::setRotation(Math::Vector3& rotation) {
    m_rotation = rotation;
}
void Engine::TransformComponent::setRotation(Math::Vector3 rotation) {
    m_rotation = rotation;
}
Engine::Math::Vector3& Engine::TransformComponent::getRotation() {
    return m_rotation;
}

void Engine::TransformComponent::update() {
    m_modelMatrix = Math::getTranslation(m_translation) * Math::getRotation(m_rotation) * Math::getScaling(m_scaling);
    m_modelMatrixInverse = Math::getScaling(1 / m_scaling) * Math::getRotation(-m_rotation) * Math::getTranslation(-m_translation);
}

Engine::Math::Matrix4& Engine::TransformComponent::getModelMatrix() {
    return m_modelMatrix;
}

Engine::Math::Matrix4& Engine::TransformComponent::getModelMatrixInverse() {
    return m_modelMatrixInverse;
}