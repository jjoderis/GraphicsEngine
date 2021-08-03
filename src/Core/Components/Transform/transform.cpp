#include "transform.h"

Engine::TransformComponent::TransformComponent()
{
    m_modelMatrix.setIdentity();
    m_modelMatrixInverse.setIdentity();
    m_normalMatrix.setIdentity();
}

void Engine::TransformComponent::translate(const Math::Vector3 &translation) { m_translation += translation; }
void Engine::TransformComponent::setTranslation(const Math::Vector3 &translation) { m_translation = translation; }
Engine::Math::Vector3 &Engine::TransformComponent::getTranslation() { return m_translation; }

void Engine::TransformComponent::scale(const Math::Vector3 &scaling) { m_scaling *= scaling; }
void Engine::TransformComponent::setScale(const Math::Vector3 &scaling) { m_scaling = scaling; }
Engine::Math::Vector3 &Engine::TransformComponent::getScaling() { return m_scaling; }

void Engine::TransformComponent::rotate(const Math::Vector3 &rotation) {
    rotate(rotation.at(0), {1, 0, 0});
    rotate(rotation.at(1), {0, 1, 0});
    rotate(rotation.at(2), {0, 0, 1});
}
void Engine::TransformComponent::setRotation(const Math::Vector3 &rotation) {
    setRotation(rotation.at(0), {1, 0, 0});
    rotate(rotation.at(1), {0, 1, 0});
    rotate(rotation.at(2), {0, 0, 1});
}
Engine::Math::Vector3 Engine::TransformComponent::getEulerRotation() {
    return Math::extractEuler(Engine::Math::getRotation(m_rotation));
}

void Engine::TransformComponent::rotate(Math::Quaternion quat) {
    quat.setUnit();
    m_rotation *= quat;
}
void Engine::TransformComponent::rotate(float angle, const Math::Vector3 &axis) { m_rotation *= Math::Quaternion{}.setRotation(axis, angle); }
void Engine::TransformComponent::setRotation(const Math::Quaternion &quat) {
    m_rotation = quat;
    m_rotation.setUnit();
}
void Engine::TransformComponent::setRotation(float angle, const Math::Vector3 &axis) { m_rotation = Math::Quaternion{}.setRotation(axis, angle); }
Engine::Math::Quaternion& Engine::TransformComponent::getRotation() { return m_rotation; }

void Engine::TransformComponent::update()
{
    m_modelMatrix = Math::getTranslation(m_translation) * Math::getRotation(m_rotation) * Math::getScaling(m_scaling);
    m_modelMatrixInverse =
        Math::getScaling(1 / m_scaling) * Math::getRotation(m_rotation.getInverse()) * Math::getTranslation(-m_translation);
    m_normalMatrix = Math::getRotation(m_rotation);
}

Engine::Math::Matrix4 &Engine::TransformComponent::getModelMatrix() { return m_modelMatrix; }

Engine::Math::Matrix4 &Engine::TransformComponent::getNormalMatrix() { return m_normalMatrix; }

Engine::Math::Matrix4 &Engine::TransformComponent::getModelMatrixInverse() { return m_modelMatrixInverse; }