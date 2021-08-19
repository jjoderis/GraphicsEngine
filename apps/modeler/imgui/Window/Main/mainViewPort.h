#ifndef APPS_MODELER_IMGUI_WINDOW_MAIN
#define APPS_MODELER_IMGUI_WINDOW_MAIN

#include "../Templates/imguiWindow.h"
#include "postProcesser.h"
#include <Core/Math/math.h>
#include <OpenGL/Framebuffer/framebuffer.h>
#include <OpenGL/Systems/RenderTracker/renderTracker.h>

namespace Engine
{
class Registry;
class OpenGLRenderer;
class CameraComponent;
class ActiveCameraComponent;
class TransformComponent;

namespace Util
{
class Ray;
class OpenGLTextureIndex;
} // namespace Util
} // namespace Engine

namespace UICreation
{

class MainViewPort : public ImGuiWindow
{

public:
    MainViewPort(Engine::Registry &registry,
                 Engine::OpenGLRenderer &renderer,
                 int &selectedEntity,
                 Engine::Util::OpenGLTextureIndex &textureIndex);

private:
    Engine::Registry &m_registry;
    Engine::Util::OpenGLTextureIndex &m_textureIndex;
    int &m_selectedEntity;
    int m_clickedEntity{-1};
    int m_grabbedEntity{-1};
    Engine::Math::IVector2 m_currentPixel{0, 0};
    Engine::Math::Vector3 m_currentPoint{0, 0, 0};

    Engine::OpenGLRenderer &m_renderer;
    std::vector<unsigned int> m_renderables{};
    Engine::Systems::OpenGLRenderTracker m_renderTracker;
    Engine::OpenGLFramebuffer m_framebuffer;

    ModelerUtil::PostProcesser m_postProcesser;

    unsigned int m_cameraEntity{0};
    std::shared_ptr<Engine::CameraComponent> m_camera{};
    std::shared_ptr<Engine::TransformComponent> m_cameraTransform{};
    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<Engine::ActiveCameraComponent>)>>
        m_cameraChangeCallback;

    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<Engine::TransformComponent>)>> m_addTransformCB;
    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<Engine::TransformComponent>)>> m_removeTransformCB;

    virtual void main();
    virtual void onResize();

    void onKeyPress(char input);

    void onMouseClick();
    void onLeftClick(const Engine::Math::IVector2 &clickedPixel);
    void onRightClick(const Engine::Math::IVector2 &clickedPixel);

    void onMouseDrag(const Engine::Math::IVector2 &dragDelta);

    void onMouseScroll(float scroll);

    void dragEntity(const Engine::Math::IVector2 &newPixel);
    void dragCamera(const Engine::Math::IVector2 &newPixel);
};

} // namespace UICreation

#endif