#include "../helpers.h"
#include <Core/Components/Light/light.h>

template <>
void UICreation::createComponentNodeMain<Engine::AmbientLightComponent>(
    std::shared_ptr<Engine::AmbientLightComponent> light, Engine::Registry &registry)
{
    ImGui::DragFloat3("Color", light->getColor().raw(), 0.01, 0.0, 1.0);
    if (ImGui::IsItemEdited())
    {
        registry.updated<Engine::AmbientLightComponent>(selectedEntity);
    }
}

template <>
void UICreation::createComponentNodeMain<Engine::DirectionalLightComponent>(
    std::shared_ptr<Engine::DirectionalLightComponent> light, Engine::Registry &registry)
{
    ImGui::DragFloat3("Color", light->getColor().raw(), 0.01, 0.0, 1.0);
    if (ImGui::IsItemEdited())
    {
        registry.updated<Engine::DirectionalLightComponent>(selectedEntity);
    }
}

template <>
void UICreation::createComponentNodeMain<Engine::PointLightComponent>(
    std::shared_ptr<Engine::PointLightComponent> light, Engine::Registry &registry)
{
    float intensity{light->getIntensity()};
    ImGui::DragFloat("Intensity", &intensity, 0.01, 0.1, 10000.0);
    if (ImGui::IsItemEdited())
    {
        light->setIntensity(intensity);
        registry.updated<Engine::PointLightComponent>(selectedEntity);
    }

    ImGui::DragFloat3("Color", light->getColor().raw(), 0.01, 0.0, 1.0);
    if (ImGui::IsItemEdited())
    {
        registry.updated<Engine::PointLightComponent>(selectedEntity);
    }
}

template <>
void UICreation::createComponentNodeMain<Engine::SpotLightComponent>(std::shared_ptr<Engine::SpotLightComponent> light,
                                                                     Engine::Registry &registry)
{
    float intensity{light->getIntensity()};
    ImGui::DragFloat("Intensity", &intensity, 0.01, 0.1, 10000.0);
    if (ImGui::IsItemEdited())
    {
        light->setIntensity(intensity);
        registry.updated<Engine::SpotLightComponent>(selectedEntity);
    }

    float cutoff{MathLib::Util::radToDeg(light->getCutoff())};
    float penumbra{MathLib::Util::radToDeg(light->getPenumbra())};
    ImGui::DragFloat("Cutoff Angle", &cutoff, 1.0, penumbra + 1.0, 175.0);
    if (ImGui::IsItemEdited())
    {
        light->setCutoff(MathLib::Util::degToRad(cutoff));
        registry.updated<Engine::SpotLightComponent>(selectedEntity);
    }

    ImGui::DragFloat("Penumbra Angle", &penumbra, 1.0, 1.0, cutoff - 1.0);
    if (ImGui::IsItemEdited())
    {
        light->setPenumbra(MathLib::Util::degToRad(penumbra));
        registry.updated<Engine::SpotLightComponent>(selectedEntity);
    }

    ImGui::DragFloat3("Color", light->getColor().raw(), 0.01, 0.0, 1.0);
    if (ImGui::IsItemEdited())
    {
        registry.updated<Engine::SpotLightComponent>(selectedEntity);
    }
}