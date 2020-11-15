#include "activeCamera.h"

Engine::OpenGLActiveCameraComponent::OpenGLActiveCameraComponent(Registry& registry) : m_registry{registry} {
    glGenBuffers(1, &m_cameraTransformUBO);

    glBindBuffer(GL_UNIFORM_BUFFER, m_cameraTransformUBO);
    glBufferData(GL_UNIFORM_BUFFER, 48 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    // wait until being added to an entity
    m_associateCallback = m_registry.onAdded<OpenGLActiveCameraComponent>([=](unsigned int addEntity, OpenGLActiveCameraComponent* addedCamera) {
        CameraComponent* camera{this->m_registry.getComponent<CameraComponent>(addEntity)};

        // TODO: remove this component if another enters the registry (only one active camera)
        // TODO: aim updateCallbacks at another camera if assigned to another entity
        if (!camera) {
            // this is not a camera => can't be an active camera => remove component
            this->m_registry.removeComponent<OpenGLActiveCameraComponent>(addEntity);
        } else {
            this->m_cameraUpdateCallback = this->m_registry.onUpdate<CameraComponent>(addEntity, [=](unsigned int updateEntity, CameraComponent* updatedCamera) {
                if (addEntity == updateEntity) {
                    this->updateCameraBuffer(updatedCamera);
                }
            });
            this->updateCameraBuffer(camera);
        }
    });
}

Engine::OpenGLActiveCameraComponent::~OpenGLActiveCameraComponent() {
    glDeleteBuffers(1, &m_cameraTransformUBO);
}

void Engine::OpenGLActiveCameraComponent::updateCameraBuffer(CameraComponent* camera) {
    glBindBuffer(GL_UNIFORM_BUFFER, m_cameraTransformUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 16 * sizeof(float), camera->getViewMatrix().raw());
    glBufferSubData(GL_UNIFORM_BUFFER, 16 * sizeof(float), 16 * sizeof(float), camera->getViewMatrixInverse().raw());
    glBufferSubData(GL_UNIFORM_BUFFER, 32 * sizeof(float), 16 * sizeof(float), camera->getProjectionMatrix().raw());    
}

void Engine::OpenGLActiveCameraComponent::bind() {
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_cameraTransformUBO);
}