#include "camera.h"

Engine::CameraComponent::CameraComponent(Registry& registry) : m_registry{registry} {
    // sets basics; camera at (0,0,0); projection project on x-y plane
    m_viewMatrix.setIdentity();
    m_projectionMatrix.setIdentity();
    m_projectionMatrix(2, 2) = 0;

    m_associateCallback = m_registry.onAdded<CameraComponent>([=](unsigned int entity, CameraComponent* camera) {
        if (this == camera) {
            this->registerEntity(entity);
        }
    });
}
 
void Engine::CameraComponent::registerEntity(unsigned int entity) {
    TransformComponent* transform{ m_registry.getComponent<TransformComponent>(entity)};

    if (transform) {
        update(entity, transform);
        setupUpdateCallback(entity);
    } else {
        awaitTransformComponent(entity);
    }
}

void Engine::CameraComponent::setupUpdateCallback(unsigned int entity) {
    m_transformUpdateCallback = m_registry.onUpdate<TransformComponent>(entity, [=](unsigned int updateEntity, TransformComponent* updatedTransform) {
        if (entity == updateEntity) {
            this->update(updateEntity, updatedTransform);
        }
    });

    m_removeTransformCallback = m_registry.onRemove<TransformComponent>([=](unsigned int removeEntity, TransformComponent* removedTransform) {
        if (entity == removeEntity) {
            m_viewMatrix.setIdentity();
            m_projectionMatrix.setIdentity();
            awaitTransformComponent(entity);
        }
    });
}

void Engine::CameraComponent::awaitTransformComponent(unsigned int entity) {
    m_transformUpdateCallback = m_registry.onAdded<TransformComponent>([=](unsigned int addEntity, TransformComponent* addedTransform) {
        if (entity == addEntity) {
            this->update(addEntity, addedTransform); 
            this->setupUpdateCallback(addEntity);
        }
    });
}

void Engine::CameraComponent::update(unsigned int entity, TransformComponent* transform) {
    m_viewMatrix = Math::getRotation(-transform->getRotation()) * Math::getTranslation(-transform->getTranslation());
    m_viewMatrixInverse = Math::getTranslation(transform->getTranslation()) * Math::getRotation(transform->getRotation());
    m_registry.updated<CameraComponent>(entity);
}

const Engine::Math::Matrix4& Engine::CameraComponent::getViewMatrix() {
    return m_viewMatrix;
}

const Engine::Math::Matrix4& Engine::CameraComponent::getViewMatrixInverse() {
    return m_viewMatrixInverse;
}

const Engine::Math::Matrix4& Engine::CameraComponent::getProjectionMatrix() {
    return m_projectionMatrix;
}