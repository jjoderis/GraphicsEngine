#include "light.h"

void drawDirectionalLightNode(Engine::Registry& registry) {
    if (std::shared_ptr<Engine::DirectionalLightComponent> light = registry.getComponent<Engine::DirectionalLightComponent>(selectedEntity)) {
        UICreation::createComponentNodeOutline<Engine::DirectionalLightComponent>("Directional Light", registry, light.get(), [&]() {
            ImGui::DragFloat3("Color", light->getColor().raw(), 0.01, 0.0, 1.0);
            if(ImGui::IsItemEdited()) {
                registry.updated<Engine::DirectionalLightComponent>(selectedEntity);
            }
        });
    }
}

void drawPointLightNode(Engine::Registry& registry) {
    if (std::shared_ptr<Engine::PointLightComponent> light = registry.getComponent<Engine::PointLightComponent>(selectedEntity)) {
        UICreation::createComponentNodeOutline<Engine::PointLightComponent>("Point Light", registry, light.get(), [&]() {
            float intensity{light->getIntensity()};
            ImGui::DragFloat("Intensity", &intensity, 0.01, 0.1, 10000.0);
            if(ImGui::IsItemEdited()) {
                light->setIntensity(intensity);
                registry.updated<Engine::PointLightComponent>(selectedEntity);
            }

            ImGui::DragFloat3("Color", light->getColor().raw(), 0.01, 0.0, 1.0);
            if(ImGui::IsItemEdited()) {
                registry.updated<Engine::PointLightComponent>(selectedEntity);
            }
        });
    }
}

void drawSpotLightNode(Engine::Registry& registry) {
    if (std::shared_ptr<Engine::SpotLightComponent> light = registry.getComponent<Engine::SpotLightComponent>(selectedEntity)) {
        UICreation::createComponentNodeOutline<Engine::SpotLightComponent>("Spot Light", registry, light.get(), [&]() {
            float intensity{light->getIntensity()};
            ImGui::DragFloat("Intensity", &intensity, 0.01, 0.1, 10000.0);
            if(ImGui::IsItemEdited()) {
                light->setIntensity(intensity);
                registry.updated<Engine::SpotLightComponent>(selectedEntity);
            }

            float cutoff{MathLib::Util::radToDeg(light->getCutoff())};
            float penumbra{MathLib::Util::radToDeg(light->getPenumbra())};
            ImGui::DragFloat("Cutoff Angle", &cutoff, 1.0, penumbra + 1.0, 175.0);
            if(ImGui::IsItemEdited()) {
                light->setCutoff(MathLib::Util::degToRad(cutoff));
                registry.updated<Engine::SpotLightComponent>(selectedEntity);
            }

            ImGui::DragFloat("Penumbra Angle", &penumbra, 1.0, 1.0, cutoff - 1.0);
            if(ImGui::IsItemEdited()) {
                light->setPenumbra(MathLib::Util::degToRad(penumbra));
                registry.updated<Engine::SpotLightComponent>(selectedEntity);
            }

            ImGui::DragFloat3("Color", light->getColor().raw(), 0.01, 0.0, 1.0);
            if(ImGui::IsItemEdited()) {
                registry.updated<Engine::SpotLightComponent>(selectedEntity);
            }
        });
    }
}

void UICreation::drawLightNodes(Engine::Registry &registry) {
    drawDirectionalLightNode(registry);
    drawPointLightNode(registry);
    drawSpotLightNode(registry);
}