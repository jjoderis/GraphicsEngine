#include "componentWindow.h"

UICreation::ComponentWindow::ComponentWindow(const std::string &name, int &currentEntity, Engine::Registry &registry) 
  : ImGuiWindow{name}, m_selectedEntity{currentEntity}, m_currentEntity{currentEntity}, m_registry{registry} 
{
}

void UICreation::ComponentWindow::checkUpdates() {
  ImGuiWindow::checkUpdates();

  if (m_currentEntity != m_selectedEntity) {
    int oldEntity = m_currentEntity;
    m_currentEntity = m_selectedEntity;
    onEntityChange(oldEntity);
  }
}

void UICreation::ComponentWindow::onEntityChange(int oldEntity) {}