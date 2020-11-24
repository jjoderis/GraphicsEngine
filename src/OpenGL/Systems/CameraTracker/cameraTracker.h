#ifndef ENGINE_OPENGL_SYSTEMS_CAMERATRACKER
#define ENGINE_OPENGL_SYSTEMS_CAMERATRACKER

#include "../../../Core/ECS/registry.h"
#include "../../../Core/Components/Camera/camera.h"
#include <glad/glad.h>

namespace Engine {
    namespace Systems {
        class OpenGLCameraTracker {
        private: 
            unsigned int& m_cameraUBO;

            // cb that makes a new camera active if there is no currently active camera or updates the buffer if the currently active camera changes
            std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<CameraComponent> camera)>> m_activeCB{};
            // cb that makes sure that only one camera is currently active
            std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<ActiveCameraComponent> activeCamera)>> m_changeActive;
            // cb that handles when the active camera is set to being inactive
            std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<ActiveCameraComponent> activeCamera)>> m_removeActive;

            int m_currentActiveCamera{-1};

            Registry& m_registry;

            void makeActiveUnique();
            void createInitActiveCB();
            void createUpdateActiveCB();

            void updateCameraBuffer(const std::shared_ptr<CameraComponent>& camera);
        public:
            OpenGLCameraTracker() = delete;

            OpenGLCameraTracker(unsigned int& cameraUBO, Registry& registry);
        };
    }
}

#endif