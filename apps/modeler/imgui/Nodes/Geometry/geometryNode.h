#ifndef APPS_MODELER_IMGUI_NODES_GEOMETRY
#define APPS_MODELER_IMGUI_NODES_GEOMETRY

#include "../helpers.h"
#include "../Templates/componentWindow.h"

namespace Engine
{
class GeometryComponent;
class Registry;
} // namespace Engine
namespace UICreation
{

class GeometryComponentWindow : public TemplatedComponentWindow<Engine::GeometryComponent> {

public:

  GeometryComponentWindow() = delete;
  GeometryComponentWindow(int &currentEntity, Engine::Registry &registry);

private: 

  virtual void main() override;

};

}

#endif