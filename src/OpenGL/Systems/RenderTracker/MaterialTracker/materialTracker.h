#ifndef ENGINE_OPENGL_SYSTEM_RENDERTRACKER_MATERIALTRACKER
#define ENGINE_OPENGL_SYSTEM_RENDERTRACKER_MATERIALTRACKER

#include <functional>
#include <memory>

namespace Engine
{
class Registry;
class OpenGLMaterialComponent;

namespace Systems
{

class OpenGLRenderMaterialTracker
{
public:
    OpenGLRenderMaterialTracker() = delete;
    OpenGLRenderMaterialTracker(const OpenGLRenderMaterialTracker &) = delete;
    OpenGLRenderMaterialTracker(OpenGLRenderMaterialTracker &&otherTracker) = delete;
    OpenGLRenderMaterialTracker(unsigned int entity, Registry &registry);

    ~OpenGLRenderMaterialTracker();

    unsigned int getBuffer();

private:
    Registry &m_registry;
    int m_materialSize{0};
    unsigned int m_UBO{0};

    using material_callback =
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<Engine::OpenGLMaterialComponent>)>>;

    material_callback m_updateCallback;

    void update(OpenGLMaterialComponent *material);
};

} // namespace Systems
} // namespace Engine

#endif