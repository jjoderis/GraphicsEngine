#ifndef ENGINE_OPENGL_COMPONENTS_CAMERA
#define ENGINE_OPENGL_COMPONENTS_CAMERA

#include "../../../Core/ECS/registry.h"
#include "../../../Core/Components/Camera/camera.h"
#include <glad/glad.h>

namespace Engine {

    class OpenGLActiveCameraComponent {
    private:
        unsigned int m_cameraTransformUBO{};
        Registry& m_registry;

        // waits for the component to be added to an entity to then set up callbacks to react to changes in the Transform of that entity
        std::shared_ptr<std::function<void(unsigned int, OpenGLActiveCameraComponent*)>> m_associateCallback;
        std::shared_ptr<std::function<void(unsigned int, CameraComponent*)>> m_cameraUpdateCallback;

        void updateCameraBuffer(CameraComponent* camera);
    public: 
        OpenGLActiveCameraComponent() = delete;
        OpenGLActiveCameraComponent(Registry& registry);
        ~OpenGLActiveCameraComponent();

        void bind();
    };

}

#endif