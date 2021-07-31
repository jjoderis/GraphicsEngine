#ifndef APPS_MODELER_IMGUI_COMPONENT_WINDOW_CLASS
#define APPS_MODELER_IMGUI_COMPONENT_WINDOW_CLASS

#include "imguiWindow.h"
#include <string>
#include <Core/ECS/registry.h>

namespace UICreation {

class ComponentWindow : public ImGuiWindow {

public:
  ComponentWindow() = delete;
  ComponentWindow(const std::string &name, int &currentEntity, Engine::Registry &registry);

protected:
  int &m_selectedEntity;
  int m_currentEntity{-1};
  Engine::Registry &m_registry;

  // extends update checks by tracking the selected entity
  virtual void checkUpdates() override;
  // allows custom handlers for entity changes in deriving classes (responsible for updating the current entity)
  virtual void onEntityChange(int oldEntity);
};

template <typename CompType>
class TemplatedComponentWindow : public ComponentWindow {

public:
  TemplatedComponentWindow() = delete;
  TemplatedComponentWindow(const std::string &name, int& currentEntity, Engine::Registry &registry)
    : ComponentWindow{name, currentEntity, registry}
  {}

protected:
  std::shared_ptr<CompType> m_component;

  virtual void render() override {
    if (m_registry.hasComponent<CompType>(m_selectedEntity)) {
      ComponentWindow::render();
    }
  }

  virtual void onEntityChange(int oldEntity) override {
    ComponentWindow::onEntityChange(oldEntity);

    checkComponentChange();  
  }

  virtual void checkUpdates() override {
    ComponentWindow::checkUpdates();

    checkComponentChange();
  }

  void checkComponentChange() {
    auto currentComponent{m_registry.getComponent<CompType>(m_currentEntity)};
    if (currentComponent != m_component) {
      auto oldComponent = m_component;
      m_component = currentComponent;
      onComponentChange(oldComponent);
    }
  }

  virtual void onComponentChange(std::shared_ptr<CompType> oldComponent) {}
};

}

#endif