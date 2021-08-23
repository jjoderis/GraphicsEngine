#include "light.h"
#include "../helpers.h"
#include <Core/Components/Light/light.h>

UICreation::LightComponentWindow::LightComponentWindow(int &currentEntity, Engine::Registry &registry)
    : ComponentWindow{"Light", currentEntity, registry}
{
}

void UICreation::LightComponentWindow::render()
{
    bool hasAmbientLight = m_registry.hasComponent<Engine::AmbientLightComponent>(m_selectedEntity);
    bool hasPointLight = m_registry.hasComponent<Engine::PointLightComponent>(m_selectedEntity);
    bool hasSpotLight = m_registry.hasComponent<Engine::SpotLightComponent>(m_selectedEntity);
    bool hasDirectionalLight = m_registry.hasComponent<Engine::DirectionalLightComponent>(m_selectedEntity);
    if (hasAmbientLight || hasPointLight || hasSpotLight || hasDirectionalLight)
    {
        ComponentWindow::render();
    }
}

void UICreation::LightComponentWindow::main()
{

    if (auto ambientLight{m_registry.getComponent<Engine::AmbientLightComponent>(m_currentEntity)})
    {
        ImGui::Text("Ambient Light");
        ImGui::SameLine();
        if (ImGui::Button("x##Ambient"))
        {
            m_registry.removeComponent<Engine::AmbientLightComponent>(m_currentEntity);
        }

        ImGui::DragFloat3("Color##Ambient", ambientLight->getColor().data(), 0.01, 0.0, 1.0);
        if (ImGui::IsItemEdited())
        {
            m_registry.updated<Engine::AmbientLightComponent>(m_currentEntity);
        }

        ImGui::Separator();
    }
    else
    {
        if (ImGui::Button("Add Ambient Light"))
        {
            m_registry.createComponent<Engine::AmbientLightComponent>(m_currentEntity);
        }
    }

    if (auto pointLight{m_registry.getComponent<Engine::PointLightComponent>(m_currentEntity)})
    {
        ImGui::Text("Point Light");
        ImGui::SameLine();
        if (ImGui::Button("x##Point"))
        {
            m_registry.removeComponent<Engine::PointLightComponent>(m_currentEntity);
        }

        float intensity{pointLight->getIntensity()};
        ImGui::DragFloat("Intensity##Point", &intensity, 0.01, 0.1, 10000.0);
        if (ImGui::IsItemEdited())
        {
            pointLight->setIntensity(intensity);
            m_registry.updated<Engine::PointLightComponent>(m_currentEntity);
        }

        ImGui::DragFloat3("Color##Point", pointLight->getColor().data(), 0.01, 0.0, 1.0);
        if (ImGui::IsItemEdited())
        {
            m_registry.updated<Engine::PointLightComponent>(m_currentEntity);
        }

        ImGui::Separator();
    }
    else
    {
        if (ImGui::Button("Add Point Light"))
        {
            m_registry.createComponent<Engine::PointLightComponent>(m_currentEntity);
        }
    }

    if (auto spotLight{m_registry.getComponent<Engine::SpotLightComponent>(m_currentEntity)})
    {
        ImGui::Text("Spot Light");
        ImGui::SameLine();
        if (ImGui::Button("x##Spot"))
        {
            m_registry.removeComponent<Engine::SpotLightComponent>(m_currentEntity);
        }

        float intensity{spotLight->getIntensity()};
        ImGui::DragFloat("Intensity##Spot", &intensity, 0.01, 0.1, 10000.0);
        if (ImGui::IsItemEdited())
        {
            spotLight->setIntensity(intensity);
            m_registry.updated<Engine::SpotLightComponent>(m_currentEntity);
        }

        float cutoff{MathLib::Util::radToDeg(spotLight->getCutoff())};
        float penumbra{MathLib::Util::radToDeg(spotLight->getPenumbra())};
        ImGui::DragFloat("Cutoff Angle##Spot", &cutoff, 1.0, penumbra + 1.0, 175.0);
        if (ImGui::IsItemEdited())
        {
            spotLight->setCutoff(MathLib::Util::degToRad(cutoff));
            m_registry.updated<Engine::SpotLightComponent>(m_currentEntity);
        }

        ImGui::DragFloat("Penumbra Angle##Spot", &penumbra, 1.0, 1.0, cutoff - 1.0);
        if (ImGui::IsItemEdited())
        {
            spotLight->setPenumbra(MathLib::Util::degToRad(penumbra));
            m_registry.updated<Engine::SpotLightComponent>(m_currentEntity);
        }

        ImGui::DragFloat3("Color##Spot", spotLight->getColor().data(), 0.01, 0.0, 1.0);
        if (ImGui::IsItemEdited())
        {
            m_registry.updated<Engine::SpotLightComponent>(m_currentEntity);
        }

        ImGui::Separator();
    }
    else
    {
        if (ImGui::Button("Add Spot Light"))
        {
            m_registry.createComponent<Engine::SpotLightComponent>(m_currentEntity);
        }
    }

    if (auto directionalLight{m_registry.getComponent<Engine::DirectionalLightComponent>(m_currentEntity)})
    {
        ImGui::Text("Directional Light");
        ImGui::SameLine();
        if (ImGui::Button("x##Directional"))
        {
            m_registry.removeComponent<Engine::DirectionalLightComponent>(m_currentEntity);
        }

        ImGui::DragFloat3("Color##Directional", directionalLight->getColor().data(), 0.01, 0.0, 1.0);
        if (ImGui::IsItemEdited())
        {
            m_registry.updated<Engine::DirectionalLightComponent>(m_currentEntity);
        }

        ImGui::Separator();
    }
    else
    {
        if (ImGui::Button("Add Directional Light"))
        {
            m_registry.createComponent<Engine::DirectionalLightComponent>(m_currentEntity);
        }
    }
}