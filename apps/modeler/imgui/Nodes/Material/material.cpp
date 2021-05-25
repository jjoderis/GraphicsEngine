#include "material.h"

void UICreation::drawMaterialNode(Engine::Registry &registry)
{
    if (std::shared_ptr<Engine::MaterialComponent> material =
            registry.getComponent<Engine::MaterialComponent>(selectedEntity))
    {
        createComponentNodeOutline<Engine::MaterialComponent>(
            "Material",
            registry,
            material.get(),
            [&]()
            {
                ImGui::ColorEdit4("Diffuse Color", material->getDiffuseColor().raw());
                if (ImGui::IsItemEdited())
                {
                    registry.updated<Engine::MaterialComponent>(selectedEntity);
                }

                ImGui::ColorEdit4("Specular Color", material->getSpecularColor().raw());
                if (ImGui::IsItemEdited())
                {
                    registry.updated<Engine::MaterialComponent>(selectedEntity);
                }

                ImGui::DragFloat("Specular Exponent", &material->getSpecularExponent());
                if (ImGui::IsItemEdited())
                {
                    registry.updated<Engine::MaterialComponent>(selectedEntity);
                }
            });
    }
}