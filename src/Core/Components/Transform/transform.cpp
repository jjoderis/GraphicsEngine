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

void Engine::TransformComponent::translate(const Vector3 &translation) { m_translation += translation; }
void Engine::TransformComponent::setTranslation(const Vector3 &translation) { m_translation = translation; }
Engine::Vector3 &Engine::TransformComponent::getTranslation() { return m_translation; }

void Engine::TransformComponent::scale(const Vector3 &scaling) { m_scaling *= scaling; }
void Engine::TransformComponent::setScale(const Vector3 &scaling) { m_scaling = scaling; }
Engine::Vector3 &Engine::TransformComponent::getScaling() { return m_scaling; }

void Engine::TransformComponent::rotate(const Vector3 &rotation)
{
    rotate(rotation.at(0), {1, 0, 0});
    rotate(rotation.at(1), {0, 1, 0});
    rotate(rotation.at(2), {0, 0, 1});
}
void Engine::TransformComponent::setRotation(const Vector3 &rotation)
{
    setRotation(rotation.at(0), {1, 0, 0});
    rotate(rotation.at(1), {0, 1, 0});
    rotate(rotation.at(2), {0, 0, 1});
}
Engine::Vector3 Engine::TransformComponent::getEulerRotation() { return extractEuler(Engine::getRotation(m_rotation)); }

void Engine::TransformComponent::rotate(Quaternion quat)
{
    quat.setUnit();
    m_rotation *= quat;
}
void Engine::TransformComponent::rotate(float angle, const Vector3 &axis)
{
    m_rotation *= Quaternion{}.setRotation(axis, angle);
}
void Engine::TransformComponent::setRotation(const Quaternion &quat)
{
    m_rotation = quat;
    m_rotation.setUnit();
}
void Engine::TransformComponent::setRotation(float angle, const Vector3 &axis)
{
    m_rotation = Quaternion{}.setRotation(axis, angle);
}
Engine::Quaternion &Engine::TransformComponent::getRotation() { return m_rotation; }

void Engine::TransformComponent::update()
{
    m_modelMatrix =
        Engine::getTranslation(m_translation) * Engine::getRotation(m_rotation) * Engine::getScaling(m_scaling);
    m_modelMatrixInverse = Engine::getScaling(1 / m_scaling) * Engine::getRotation(m_rotation.getInverse()) *
                           Engine::getTranslation(-m_translation);

    m_matrixWorld = m_modelMatrix;
    m_matrixWorldInverse = m_modelMatrixInverse;

    m_normalMatrix = Engine::getRotation(m_rotation);
    m_normalMatrixInverse = Engine::getRotation(m_rotation.getInverse());

    m_normalMatrixWorld = m_normalMatrix;
    m_normalMatrixWorldInverse = m_normalMatrixInverse;

    m_viewMatrix = Engine::getRotation(m_rotation.getInverse()) * Engine::getTranslation(-m_translation);
    m_viewMatrixInverse = Engine::getTranslation(m_translation) * Engine::getRotation(m_rotation);

    m_viewMatrixWorld = m_viewMatrix;
    m_viewMatrixWorldInverse = m_viewMatrixInverse;
}

Engine::Matrix4 &Engine::TransformComponent::getModelMatrix() { return m_modelMatrix; }
Engine::Matrix4 &Engine::TransformComponent::getModelMatrixInverse() { return m_modelMatrixInverse; }

Engine::Matrix4 &Engine::TransformComponent::getMatrixWorld() { return m_matrixWorld; }
Engine::Matrix4 &Engine::TransformComponent::getMatrixWorldInverse() { return m_matrixWorldInverse; }

Engine::Matrix4 &Engine::TransformComponent::getNormalMatrix() { return m_normalMatrix; }
Engine::Matrix4 &Engine::TransformComponent::getNormalMatrixInverse() { return m_normalMatrixInverse; }

Engine::Matrix4 &Engine::TransformComponent::getNormalMatrixWorld() { return m_normalMatrixWorld; }
Engine::Matrix4 &Engine::TransformComponent::getNormalMatrixWorldInverse() { return m_normalMatrixWorldInverse; }

Engine::Matrix4 &Engine::TransformComponent::getViewMatrix() { return m_viewMatrix; }
Engine::Matrix4 &Engine::TransformComponent::getViewMatrixInverse() { return m_viewMatrixInverse; }

Engine::Matrix4 &Engine::TransformComponent::getViewMatrixWorld() { return m_viewMatrixWorld; }
Engine::Matrix4 &Engine::TransformComponent::getViewMatrixWorldInverse() { return m_viewMatrixWorldInverse; }