#include "camera.h"

void UICreation::drawCameraNode(Engine::Registry& registry) {
    Engine::CameraComponent* camera = registry.getComponent<Engine::CameraComponent>(selectedEntity);

    createComponentNodeOutline<Engine::CameraComponent>("Camera", registry, camera, [&]() {
        ImGui::DragFloat("Near", &camera->getNear(), 0.1f, 0.1f, camera->getFar() - 0.1);
        if(ImGui::IsItemEdited()) {
            camera->calculateProjection();
            registry.updated<Engine::CameraComponent>(selectedEntity);
        }
        ImGui::DragFloat("Far", &camera->getFar(), 0.1f, camera->getNear() + 0.1f, 100000.0);
        if(ImGui::IsItemEdited()) {
            camera->calculateProjection();
            registry.updated<Engine::CameraComponent>(selectedEntity);
        }
        ImGui::DragFloat("FOV", &camera->getFov(), 0.1f, M_PI_4 / 2, M_PI * 0.8);
        if(ImGui::IsItemEdited()) {
            camera->calculateProjection();
            registry.updated<Engine::CameraComponent>(selectedEntity);
        }
    });
}