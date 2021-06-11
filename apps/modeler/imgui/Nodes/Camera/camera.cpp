#include "camera.h"

#include <Core/Components/Camera/camera.h>

template <>
void UICreation::createComponentNodeMain<Engine::CameraComponent>(std::shared_ptr<Engine::CameraComponent> camera,
                                                                  Engine::Registry &registry)
{
    ImGui::DragFloat("Near", &camera->getNear(), 0.1f, 0.1f, camera->getFar() - 0.1);
    if (ImGui::IsItemEdited())
    {
        camera->calculateProjection();
        registry.updated<Engine::CameraComponent>(selectedEntity);
    }
    ImGui::DragFloat("Far", &camera->getFar(), 0.1f, camera->getNear() + 0.1f, 100000.0);
    if (ImGui::IsItemEdited())
    {
        camera->calculateProjection();
        registry.updated<Engine::CameraComponent>(selectedEntity);
    }
    ImGui::DragFloat("FOV", &camera->getFov(), 0.1f, M_PI_4 / 2, M_PI * 0.8);
    if (ImGui::IsItemEdited())
    {
        camera->calculateProjection();
        registry.updated<Engine::CameraComponent>(selectedEntity);
    }
}