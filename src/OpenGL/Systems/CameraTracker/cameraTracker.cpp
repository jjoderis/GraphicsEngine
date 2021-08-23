#include "cameraTracker.h"

#include "../../../Core/Components/Camera/camera.h"
#include "../../../Core/Components/Transform/transform.h"
#include "../../../Core/ECS/registry.h"

const float baseCameraTransforms[48]{
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // base viewMatrix (camera at (0, 0, 0))
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // base viewMatrixInverse
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // base projection (ortographic onto z plane)
};

Engine::Systems::OpenGLCameraTracker::OpenGLCameraTracker(unsigned int &cameraUBO, Registry &registry)
    : m_cameraUBO{cameraUBO}, m_registry{registry}
{
    glGenBuffers(1, &m_cameraUBO);

    glBindBuffer(GL_UNIFORM_BUFFER, m_cameraUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(baseCameraTransforms), baseCameraTransforms, GL_DYNAMIC_DRAW);

    m_changeActive = m_registry.onAdded<ActiveCameraComponent>(
        [this](unsigned int entity, std::weak_ptr<ActiveCameraComponent> active)
        {
            if (auto camera{this->m_registry.getComponent<CameraComponent>(entity)})
            {
                // camera can only be activated if it is a camera
                this->m_currentActiveCamera = entity;
                this->updateCameraBuffer(camera);
                this->createUpdateActiveCB();

                if (auto transform{this->m_registry.getComponent<Engine::TransformComponent>(entity)})
                {
                    this->updateCameraBufferTransform(transform->getViewMatrixWorld(),
                                                      transform->getViewMatrixWorldInverse());
                }
                else
                {
                    this->updateCameraBufferTransform(Engine::Matrix4{}.setIdentity(), Engine::Matrix4{}.setIdentity());
                }
            }

            // make active token unique for currently active camera (no other entity is allowed to hold a active camera
            // token)
            this->makeActiveUnique();
        });

    createInitActiveCB();

    m_removeActive = m_registry.onRemove<ActiveCameraComponent>(
        [this](unsigned int entity, std::weak_ptr<ActiveCameraComponent> active)
        {
            if (this->m_currentActiveCamera == entity)
            {
                glBindBuffer(GL_UNIFORM_BUFFER, this->m_cameraUBO);
                glBufferData(GL_UNIFORM_BUFFER, sizeof(baseCameraTransforms), baseCameraTransforms, GL_DYNAMIC_DRAW);
                this->m_currentActiveCamera = -1;
                this->createInitActiveCB();
            }
        });

    m_addTransformCB = m_registry.onAdded<TransformComponent>(
        [this](unsigned int entity, std::weak_ptr<TransformComponent> transform)
        {
            if (this->m_currentActiveCamera == entity)
            {
                auto locked{transform.lock()};
                this->updateCameraBufferTransform(locked->getViewMatrixWorld(), locked->getViewMatrixWorldInverse());
            }
        });

    m_updateTransformCB = m_registry.onUpdate<TransformComponent>(
        [this](unsigned int entity, std::weak_ptr<TransformComponent> transform)
        {
            if (this->m_currentActiveCamera == entity)
            {
                auto locked{transform.lock()};
                this->updateCameraBufferTransform(locked->getViewMatrixWorld(), locked->getViewMatrixWorldInverse());
            }
        });

    m_removeTransformCB = m_registry.onRemove<TransformComponent>(
        [this](unsigned int entity, std::weak_ptr<TransformComponent> transform)
        {
            if (this->m_currentActiveCamera == entity)
            {
                this->updateCameraBufferTransform(Engine::Matrix4{}.setIdentity(), Engine::Matrix4{}.setIdentity());
            }
        });
}

void Engine::Systems::OpenGLCameraTracker::createInitActiveCB()
{
    m_activeCB = m_registry.onAdded<CameraComponent>(
        [this](unsigned int entity, std::weak_ptr<CameraComponent> camera)
        {
            if (m_registry.getComponents<Engine::CameraComponent>().size() == 1)
            {
                m_registry.createComponent<Engine::ActiveCameraComponent>(entity);
            }
        });
}

void Engine::Systems::OpenGLCameraTracker::createUpdateActiveCB()
{
    m_activeCB = m_registry.onUpdate<CameraComponent>(m_currentActiveCamera,
                                                      [this](unsigned int entity, std::weak_ptr<CameraComponent> camera)
                                                      { this->updateCameraBuffer(camera.lock()); });
}

void Engine::Systems::OpenGLCameraTracker::makeActiveUnique()
{
    std::vector<std::shared_ptr<ActiveCameraComponent>> activeCameras{
        m_registry.getComponents<ActiveCameraComponent>()};

    std::vector<std::shared_ptr<ActiveCameraComponent>>::const_reverse_iterator it = activeCameras.rbegin();
    std::vector<std::shared_ptr<ActiveCameraComponent>>::const_reverse_iterator endIt = activeCameras.rend();
    // remove all ActiveCameraComponents that are not the current one
    while (it != activeCameras.rend())
    {
        std::list<unsigned int> owners{m_registry.getOwners<ActiveCameraComponent>(*it)};

        std::list<unsigned int>::const_reverse_iterator ownerIt = owners.rbegin();

        while (ownerIt != owners.rend())
        {
            if (*ownerIt != m_currentActiveCamera)
            {
                m_registry.removeComponent<ActiveCameraComponent>(*ownerIt);
            }

            ++ownerIt;
        }

        ++it;
    }
}

void Engine::Systems::OpenGLCameraTracker::updateCameraBuffer(const std::shared_ptr<CameraComponent> &camera)
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_cameraUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 32 * sizeof(float), 16 * sizeof(float), camera->getProjectionMatrix().raw());
}

void Engine::Systems::OpenGLCameraTracker::updateCameraBufferTransform(Engine::Matrix4 &viewMatrix,
                                                                       Engine::Matrix4 &viewMatrixInverse)
{
    glBindBuffer(GL_UNIFORM_BUFFER, m_cameraUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 16 * sizeof(float), viewMatrix.raw());
    glBufferSubData(GL_UNIFORM_BUFFER, 16 * sizeof(float), 16 * sizeof(float), viewMatrixInverse.raw());
}