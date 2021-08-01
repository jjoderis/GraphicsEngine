#ifndef ENGINE_OPENGL_COMPONENTS_RENDERER
#define ENGINE_OPENGL_COMPONENTS_RENDERER

#include "../Systems/CameraTracker/cameraTracker.h"
#include "../Systems/LightsTracker/lightsTracker.h"
#include <glad/glad.h>

namespace Engine
{
class Registry;

class OpenGLFramebuffer {

public:

    OpenGLFramebuffer();
    ~OpenGLFramebuffer();

    void bind();
    void unbind();
    unsigned int getTexture();
    void clear();

    void resize(int width, int height);

private:
    unsigned int m_framebuffer{0};
    unsigned int m_texture{0};
    unsigned int m_depthStencil{0};
    int m_width{800};
    int m_height{600};
};

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

    void render(const std::vector<unsigned int> &renderables);
};
} // namespace Engine

#endif