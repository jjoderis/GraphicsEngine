#include "imguiWindow.h"

#include <imgui.h>
#include <iostream>

UICreation::ImGuiWindow::ImGuiWindow(const std::string &name) : m_name{name} {}

void UICreation::ImGuiWindow::render() {
  if (start()) {
    checkUpdates();
    main();
  }

  end();
}

bool UICreation::ImGuiWindow::start() {
  return ImGui::Begin(m_name.c_str(), &m_open, m_flags);
}

void UICreation::ImGuiWindow::main() {
  ImGui::Text("Implement Me");
}

void UICreation::ImGuiWindow::end() {
  ImGui::End();
}

void UICreation::ImGuiWindow::checkUpdates() {
  auto size{ImGui::GetWindowSize()};
  if (size.x != m_width || size.y != m_height) {
    m_width = size.x;
    m_height = size.y;

    onResize();
  }
}

void UICreation::ImGuiWindow::onResize() {}