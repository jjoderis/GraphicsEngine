#ifndef APPS_MODELER_IMGUI_NODES_TRANSFORM
#define APPS_MODELER_IMGUI_NODES_TRANSFORM

#include "../helpers.h"
#include "../Templates/componentWindow.h"

namespace Engine
{
class TransformComponent;
class Registry;
} // namespace Engine
namespace UICreation
{

class TransformComponentWindow : public TemplatedComponentWindow<Engine::TransformComponent> {

public:

  TransformComponentWindow() = delete;
  TransformComponentWindow(int &currentEntity, Engine::Registry &registry);

private:

  virtual void main() override;

};

}

#endif