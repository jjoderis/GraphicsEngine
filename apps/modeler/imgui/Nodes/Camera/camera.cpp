#include "camera.h"

#include <Core/Components/Camera/camera.h>

UICreation::CameraComponentWindow::CameraComponentWindow(int &currentEntity, Engine::Registry &registry)
    : TemplatedComponentWindow<Engine::CameraComponent>{"Camera", currentEntity, registry}
{
}

void UICreation::CameraComponentWindow::main() {
    ImGui::DragFloat("Near", &m_component->getNear(), 0.1f, 0.1f, m_component->getFar() - 0.1);
    if (ImGui::IsItemEdited())
    {
        m_component->calculateProjection();
        m_registry.updated<Engine::CameraComponent>(selectedEntity);
    }
    ImGui::DragFloat("Far", &m_component->getFar(), 0.1f, m_component->getNear() + 0.1f, 100000.0);
    if (ImGui::IsItemEdited())
    {
        m_component->calculateProjection();
        m_registry.updated<Engine::CameraComponent>(selectedEntity);
    }
    ImGui::DragFloat("FOV", &m_component->getFov(), 0.1f, M_PI_4 / 2, M_PI * 0.8);
    if (ImGui::IsItemEdited())
    {
        m_component->calculateProjection();
        m_registry.updated<Engine::CameraComponent>(selectedEntity);
    }
}