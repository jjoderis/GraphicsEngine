#ifndef ENGINE_OPENGL_COMPONENTS_RENDERER
#define ENGINE_OPENGL_COMPONENTS_RENDERER

#include <glad/glad.h>
#include "../../Core/ECS/registry.h"
#include "../Components/Render/render.h"
#include "../Systems/CameraTracker/cameraTracker.h"
#include "../Systems/LightsTracker/lightsTracker.h"

namespace Engine {
    class OpenGLRenderer {
    private:
        unsigned int m_lightsInfoUBO{0};
        unsigned int m_activeCameraUBO{0};

        Systems::OpenGLCameraTracker m_cameraTracker;
        Systems::OpenGLLightsTracker m_lightsTracker;

        Registry& m_registry;
    public:
        OpenGLRenderer() = delete;
        OpenGLRenderer(Registry& registry);
        ~OpenGLRenderer();

        void render();
    };
}

#endif