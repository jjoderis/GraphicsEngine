#include "imguiWindow.h"

#include <imgui.h>
#include <iostream>

UICreation::ImGuiWindow::ImGuiWindow(const std::string &name) : m_name{name} {}

void UICreation::ImGuiWindow::render()
{
    if (start())
    {
        checkUpdates();
        main();
    }

    end();
}

bool UICreation::ImGuiWindow::start()
{
    ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
    return ImGui::Begin(m_name.c_str(), &m_open, m_flags);
}

void UICreation::ImGuiWindow::main() { ImGui::Text("Implement Me"); }

void UICreation::ImGuiWindow::end() { ImGui::End(); }

void UICreation::ImGuiWindow::checkUpdates()
{
    Engine::Math::IVector2 size{ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y};
    auto windowPos{ImGui::GetWindowPos()};
    auto contentOffset{ImGui::GetWindowContentRegionMin()};
    Engine::Math::IVector2 pos{windowPos.x + contentOffset.x, windowPos.y + contentOffset.y};

    if (size != m_size)
    {
        m_size = size;

        onResize();
    }

    if (pos != m_pos)
    {
        m_pos = pos;

        // TODO: onMove();
    }
}

void UICreation::ImGuiWindow::onResize() {}