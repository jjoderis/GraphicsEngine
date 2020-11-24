#include "material.h"

void UICreation::drawMaterialNode(Engine::Registry &registry) {
    Engine::MaterialComponent* material = registry.getComponent<Engine::MaterialComponent>(selectedEntity);

    createComponentNodeOutline<Engine::MaterialComponent>("Material", registry, material, [&]() {
        ImGui::ColorEdit4("Color", material->getColor().raw());

        if (ImGui::IsItemEdited()) {
            registry.updated<Engine::MaterialComponent>(selectedEntity);
        }
    });
}