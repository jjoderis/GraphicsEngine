#ifndef ENGINE_OPENGL_COMPONENTS_RENDERER
#define ENGINE_OPENGL_COMPONENTS_RENDERER

#include "../Systems/CameraTracker/cameraTracker.h"
#include "../Systems/LightsTracker/lightsTracker.h"
#include <glad/glad.h>

namespace Engine
{
class Registry;

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

    Registry &m_registry;

public:
    OpenGLRenderer() = delete;
    OpenGLRenderer(Registry &registry);
    ~OpenGLRenderer();

    unsigned int &getCameraUBO();

    void render(const std::vector<unsigned int> &renderables);
};
} // namespace Engine

#endif