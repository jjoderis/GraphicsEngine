#include "material.h"

void UICreation::drawMaterialNode(Engine::Registry &registry) {
    if (std::shared_ptr<Engine::MaterialComponent> material = registry.getComponent<Engine::MaterialComponent>(selectedEntity)) {
        createComponentNodeOutline<Engine::MaterialComponent>("Material", registry, material.get(), [&]() {
            ImGui::ColorEdit4("Color", material->getColor().raw());

            if (ImGui::IsItemEdited()) {
                registry.updated<Engine::MaterialComponent>(selectedEntity);
            }
        });
    }
}