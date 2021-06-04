#ifndef ENGINE_OPENGL_COMPONENTS_RENDERER
#define ENGINE_OPENGL_COMPONENTS_RENDERER

#include "../../Core/ECS/registry.h"
#include "../Systems/CameraTracker/cameraTracker.h"
#include "../Systems/LightsTracker/lightsTracker.h"
#include "../Systems/RenderTracker/renderTracker.h"
#include <glad/glad.h>

namespace Engine
{
class OpenGLRenderer
{
private:
    unsigned int m_ambientLightsInfoUBO{0};
    unsigned int m_directionalLightsInfoUBO{0};
    unsigned int m_pointLightsInfoUBO{0};
    unsigned int m_spotLightsInfoUBO{0};
    unsigned int m_activeCameraUBO{0};

    Systems::OpenGLCameraTracker m_cameraTracker;
    Systems::AmbientLightsTracker m_ambientLightsTracker;
    Systems::DirectionalLightsTracker m_directionalLightsTracker;
    Systems::PointLightsTracker m_pointLightsTracker;
    Systems::SpotLightsTracker m_spotLightsTracker;
    Systems::OpenGLRenderTracker m_renderTracker;

    Registry &m_registry;

public:
    OpenGLRenderer() = delete;
    OpenGLRenderer(Registry &registry);
    ~OpenGLRenderer();

    void render();
};
} // namespace Engine

#endif