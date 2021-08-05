#include "camera.h"

#include <Core/Components/Camera/camera.h>
#include <imgui.h>

UICreation::CameraComponentWindow::CameraComponentWindow(int &currentEntity, Engine::Registry &registry)
    : TemplatedComponentWindow<Engine::CameraComponent>{"Camera", currentEntity, registry}
{
}

void UICreation::CameraComponentWindow::main() {
    ImGui::DragFloat("Near", &m_component->getNear(), 0.1f, 0.1f, m_component->getFar() - 0.1);
    if (ImGui::IsItemEdited())
    {
        m_component->calculateProjection();
        m_registry.updated<Engine::CameraComponent>(m_selectedEntity);
    }
    ImGui::DragFloat("Far", &m_component->getFar(), 0.1f, m_component->getNear() + 0.1f, 100000.0);
    if (ImGui::IsItemEdited())
    {
        m_component->calculateProjection();
        m_registry.updated<Engine::CameraComponent>(m_selectedEntity);
    }
    ImGui::DragFloat("FOV", &m_component->getFov(), 0.1f, M_PI_4 / 2, M_PI * 0.8);
    if (ImGui::IsItemEdited())
    {
        m_component->calculateProjection();
        m_registry.updated<Engine::CameraComponent>(m_selectedEntity);
    }
    auto isUsed = m_registry.hasComponent<Engine::ActiveCameraComponent>(m_currentEntity);
    if (ImGui::Checkbox("Use Camera", &isUsed) && isUsed) {
        unsigned int activeCameraEntity = m_registry.getOwners<Engine::ActiveCameraComponent>()[0].front();
        auto oldCamera = m_registry.getComponent<Engine::CameraComponent>(activeCameraEntity);
        m_component->setAspect(oldCamera->getAspect());
        m_component->calculateProjection();
        m_registry.updated<Engine::CameraComponent>(m_currentEntity);
        m_registry.createComponent<Engine::ActiveCameraComponent>(m_currentEntity);
    }
}