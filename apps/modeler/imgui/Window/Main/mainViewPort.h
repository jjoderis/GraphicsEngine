#ifndef APPS_MODELER_IMGUI_WINDOW_MAIN
#define APPS_MODELER_IMGUI_WINDOW_MAIN

#include "../Templates/imguiWindow.h"
#include <OpenGL/Systems/RenderTracker/renderTracker.h>
#include <OpenGL/Renderer/renderer.h>
#include <Core/Math/math.h>

namespace Engine {
  class Registry;
}

namespace UICreation {

class MainViewPort : public ImGuiWindow {

public:

  MainViewPort(Engine::Registry &registry, Engine::OpenGLRenderer &renderer, int &selectedEntity);

private:
  Engine::Registry &m_registry;
  int &m_selectedEntity;
  int m_grabbedEntity{-1};
  float m_entityDistance{0.0f};
  Engine::Math::IVector2 m_currentPixel{0, 0};
  Engine::Math::Vector3 m_currentPoint{0, 0, 0};


  Engine::OpenGLRenderer &m_renderer;
  std::vector<unsigned int> m_renderables{};
  Engine::Systems::OpenGLRenderTracker m_renderTracker;
  Engine::OpenGLFramebuffer m_framebuffer{};

  virtual void main();
  virtual void onResize();
};

}

#endif