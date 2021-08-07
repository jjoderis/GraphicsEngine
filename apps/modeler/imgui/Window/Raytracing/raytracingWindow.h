#ifndef APPS_MODELER_IMGUI_WINDOW_RAYTRACING
#define APPS_MODELER_IMGUI_WINDOW_RAYTRACING

#include "../Templates/imguiWindow.h"

namespace Engine
{
class Registry;
}

namespace UICreation
{

class RaytracingViewport : public ImGuiWindow {

public: 

  RaytracingViewport() = delete;
  RaytracingViewport(Engine::Registry &registry);

  void newFrame();

private:
  Engine::Registry &m_registry;
  unsigned int m_texture{0};

  virtual void main();

};

} // namespace UICreation

#endif