#ifndef APPS_MODELER_IMGUI_NODES_MATERIAL
#define APPS_MODELER_IMGUI_NODES_MATERIAL

#include "../helpers.h"
#include "../Templates/componentWindow.h"

namespace Engine
{
class Registry;
} // namespace Engine
namespace UICreation
{

class MaterialComponentWindow : public ComponentWindow {

public:

  MaterialComponentWindow() = delete;
  MaterialComponentWindow(int &currentEntity, Engine::Registry &registry);

  virtual void render() override;

private: 

  virtual void main() override;

};

}

#endif