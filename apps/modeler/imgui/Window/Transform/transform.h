#ifndef APPS_MODELER_IMGUI_NODES_TRANSFORM
#define APPS_MODELER_IMGUI_NODES_TRANSFORM

#include "../Templates/componentWindow.h"
#include <Core/Math/math.h>

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
  Engine::Math::Vector3 m_euler{0, 0, 0};
  Engine::Math::Vector4 m_axisAngle{0, 0, 0, 0};
  Engine::Math::Quaternion m_quat{0.0, 0.0, 0.0, 1.0};
  
  void updateInternals();

  virtual void main() override;

  void drawEuler();
  void drawAngleAxis();
  void drawQuaternion();

  virtual void onComponentChange(std::shared_ptr<Engine::TransformComponent> oldComponent) override;

  std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<Engine::TransformComponent>)>> m_transformChangeCallback;

};

}

#endif