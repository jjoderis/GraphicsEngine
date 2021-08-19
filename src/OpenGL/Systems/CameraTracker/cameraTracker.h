#ifndef ENGINE_OPENGL_SYSTEMS_CAMERATRACKER
#define ENGINE_OPENGL_SYSTEMS_CAMERATRACKER

#include "../../../Core/Math/math.h"
#include <functional>
#include <glad/glad.h>
#include <memory>

namespace Engine
{
class Registry;
class CameraComponent;
class ActiveCameraComponent;
class TransformComponent;
namespace Systems
{
class OpenGLCameraTracker
{
private:
    unsigned int &m_cameraUBO;

    // cb that makes a new camera active if there is no currently active camera or updates the buffer if the currently
    // active camera changes
    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<CameraComponent> camera)>> m_activeCB{};
    // cb that makes sure that only one camera is currently active
    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<ActiveCameraComponent> activeCamera)>>
        m_changeActive;
    // cb that handles when the active camera is set to being inactive
    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<ActiveCameraComponent> activeCamera)>>
        m_removeActive;

    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>> m_addTransformCB;
    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>> m_updateTransformCB;
    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>> m_removeTransformCB;

    int m_currentActiveCamera{-1};

    Registry &m_registry;

    void makeActiveUnique();
    void createInitActiveCB();
    void createUpdateActiveCB();

    void awaitTransform();
    void setupTransformUpdateCallback();

    void updateCameraBuffer(const std::shared_ptr<CameraComponent> &camera);

    void updateCameraBufferTransform(Engine::Math::Matrix4 &viewMatrix, Engine::Math::Matrix4 &viewMatrixInverse);

public:
    OpenGLCameraTracker() = delete;

    OpenGLCameraTracker(unsigned int &cameraUBO, Registry &registry);
};
} // namespace Systems
} // namespace Engine

#endif