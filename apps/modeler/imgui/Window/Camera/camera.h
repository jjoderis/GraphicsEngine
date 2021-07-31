#ifndef APPS_MODELER_IMGUI_NODES_CAMERA
#define APPS_MODELER_IMGUI_NODES_CAMERA

#include "../Templates/componentWindow.h"

namespace Engine
{
class CameraComponent;
class Registry;
} // namespace Engine
namespace UICreation
{

class CameraComponentWindow : public TemplatedComponentWindow<Engine::CameraComponent> {

public:

  CameraComponentWindow() = delete;
  CameraComponentWindow(int &currentEntity, Engine::Registry &registry);

private: 

  virtual void main() override;

};

}

#endif