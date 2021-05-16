#include "cameraTracker.h"

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
            std::shared_ptr<CameraComponent> camera{this->m_registry.getComponent<CameraComponent>(entity)};

            if (camera)
            {
                // camera can only be activated if it is a camera
                this->m_currentActiveCamera = entity;
                this->updateCameraBuffer(camera);
                this->createUpdateActiveCB();
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
}

void Engine::Systems::OpenGLCameraTracker::createInitActiveCB()
{
    m_activeCB = m_registry.onAdded<CameraComponent>(
        [this](unsigned int entity, std::weak_ptr<CameraComponent> camera)
        {
            if (m_registry.getComponents<Engine::CameraComponent>().size() == 1)
            {
                m_registry.addComponent<Engine::ActiveCameraComponent>(
                    entity, std::make_shared<Engine::ActiveCameraComponent>());
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
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 16 * sizeof(float), camera->getViewMatrix().raw());
    glBufferSubData(GL_UNIFORM_BUFFER, 16 * sizeof(float), 16 * sizeof(float), camera->getViewMatrixInverse().raw());
    glBufferSubData(GL_UNIFORM_BUFFER, 32 * sizeof(float), 16 * sizeof(float), camera->getProjectionMatrix().raw());
}