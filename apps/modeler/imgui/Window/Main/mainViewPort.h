#ifndef APPS_MODELER_IMGUI_WINDOW_MAIN
#define APPS_MODELER_IMGUI_WINDOW_MAIN

#include "../Templates/imguiWindow.h"
#include <OpenGL/Systems/RenderTracker/renderTracker.h>
#include <OpenGL/Renderer/renderer.h>
#include <Core/Math/math.h>
#include "postProcesser.h"

namespace Engine {
  class Registry;

  namespace Util {
    class Ray;
  }
}

namespace UICreation {

class MainViewPort : public ImGuiWindow {

public:

  MainViewPort(Engine::Registry &registry, Engine::OpenGLRenderer &renderer, int &selectedEntity);

private:
  Engine::Registry &m_registry;
  int &m_selectedEntity;
  int m_grabbedEntity{-1};
  Engine::Math::IVector2 m_currentPixel{0, 0};
  Engine::Math::Vector3 m_currentPoint{0, 0, 0};

  Engine::OpenGLRenderer &m_renderer;
  std::vector<unsigned int> m_renderables{};
  Engine::Systems::OpenGLRenderTracker m_renderTracker;
  Engine::OpenGLFramebuffer m_framebuffer{};
  
  ModelerUtil::PostProcesser m_postProcesser{};

  unsigned int m_cameraEntity{0};
  std::shared_ptr<Engine::CameraComponent> m_camera{};
  std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<Engine::ActiveCameraComponent>)>> m_cameraChangeCallback;

  virtual void main();
  virtual void onResize();

  void dragEntity(const Engine::Math::IVector2 &newPixel);
  void dragCamera(const Engine::Math::IVector2 &newPixel);
};

}

#endif