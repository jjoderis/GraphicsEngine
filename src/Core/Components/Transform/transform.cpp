#include "transform.h"

Engine::TransformComponent::TransformComponent()
{
    m_modelMatrix.setIdentity();
    m_modelMatrixInverse.setIdentity();

    m_matrixWorld.setIdentity();
    m_matrixWorldInverse.setIdentity();

    m_normalMatrix.setIdentity();
    m_normalMatrixInverse.setIdentity();

    m_normalMatrixWorld.setIdentity();
    m_normalMatrixWorldInverse.setIdentity();

    m_viewMatrix.setIdentity();
    m_viewMatrixInverse.setIdentity();

    m_viewMatrixWorld.setIdentity();
    m_viewMatrixWorldInverse.setIdentity();
}

void Engine::TransformComponent::translate(const Math::Vector3 &translation) { m_translation += translation; }
void Engine::TransformComponent::setTranslation(const Math::Vector3 &translation) { m_translation = translation; }
Engine::Math::Vector3 &Engine::TransformComponent::getTranslation() { return m_translation; }

void Engine::TransformComponent::scale(const Math::Vector3 &scaling) { m_scaling *= scaling; }
void Engine::TransformComponent::setScale(const Math::Vector3 &scaling) { m_scaling = scaling; }
Engine::Math::Vector3 &Engine::TransformComponent::getScaling() { return m_scaling; }

void Engine::TransformComponent::rotate(const Math::Vector3 &rotation)
{
    rotate(rotation.at(0), {1, 0, 0});
    rotate(rotation.at(1), {0, 1, 0});
    rotate(rotation.at(2), {0, 0, 1});
}
void Engine::TransformComponent::setRotation(const Math::Vector3 &rotation)
{
    setRotation(rotation.at(0), {1, 0, 0});
    rotate(rotation.at(1), {0, 1, 0});
    rotate(rotation.at(2), {0, 0, 1});
}
Engine::Math::Vector3 Engine::TransformComponent::getEulerRotation()
{
    return Math::extractEuler(Engine::Math::getRotation(m_rotation));
}

void Engine::TransformComponent::rotate(Math::Quaternion quat)
{
    quat.setUnit();
    m_rotation *= quat;
}
void Engine::TransformComponent::rotate(float angle, const Math::Vector3 &axis)
{
    m_rotation *= Math::Quaternion{}.setRotation(axis, angle);
}
void Engine::TransformComponent::setRotation(const Math::Quaternion &quat)
{
    m_rotation = quat;
    m_rotation.setUnit();
}
void Engine::TransformComponent::setRotation(float angle, const Math::Vector3 &axis)
{
    m_rotation = Math::Quaternion{}.setRotation(axis, angle);
}
Engine::Math::Quaternion &Engine::TransformComponent::getRotation() { return m_rotation; }

void Engine::TransformComponent::update()
{
    m_modelMatrix = Math::getTranslation(m_translation) * Math::getRotation(m_rotation) * Math::getScaling(m_scaling);
    m_modelMatrixInverse = Math::getScaling(1 / m_scaling) * Math::getRotation(m_rotation.getInverse()) *
                           Math::getTranslation(-m_translation);

    m_matrixWorld = m_modelMatrix;
    m_matrixWorldInverse = m_modelMatrixInverse;

    m_normalMatrix = Math::getRotation(m_rotation);
    m_normalMatrixInverse = Math::getRotation(m_rotation.getInverse());

    m_normalMatrixWorld = m_normalMatrix;
    m_normalMatrixWorldInverse = m_normalMatrixInverse;

    m_viewMatrix = Math::getRotation(m_rotation.getInverse()) * Math::getTranslation(-m_translation);
    m_viewMatrixInverse = Math::getTranslation(m_translation) * Math::getRotation(m_rotation);

    m_viewMatrixWorld = m_viewMatrix;
    m_viewMatrixWorldInverse = m_viewMatrixInverse;
}

Engine::Math::Matrix4 &Engine::TransformComponent::getModelMatrix() { return m_modelMatrix; }
Engine::Math::Matrix4 &Engine::TransformComponent::getModelMatrixInverse() { return m_modelMatrixInverse; }

Engine::Math::Matrix4 &Engine::TransformComponent::getMatrixWorld() { return m_matrixWorld; }
Engine::Math::Matrix4 &Engine::TransformComponent::getMatrixWorldInverse() { return m_matrixWorldInverse; }

Engine::Math::Matrix4 &Engine::TransformComponent::getNormalMatrix() { return m_normalMatrix; }
Engine::Math::Matrix4 &Engine::TransformComponent::getNormalMatrixInverse() { return m_normalMatrixInverse; }

Engine::Math::Matrix4 &Engine::TransformComponent::getNormalMatrixWorld() { return m_normalMatrixWorld; }
Engine::Math::Matrix4 &Engine::TransformComponent::getNormalMatrixWorldInverse() { return m_normalMatrixWorldInverse; }

Engine::Math::Matrix4 &Engine::TransformComponent::getViewMatrix() { return m_viewMatrix; }
Engine::Math::Matrix4 &Engine::TransformComponent::getViewMatrixInverse() { return m_viewMatrixInverse; }

Engine::Math::Matrix4 &Engine::TransformComponent::getViewMatrixWorld() { return m_viewMatrixWorld; }
Engine::Math::Matrix4 &Engine::TransformComponent::getViewMatrixWorldInverse() { return m_viewMatrixWorldInverse; }