#include "../helpers.h"
#include "openGLMaterial.h"
#include <OpenGL/Components/Material/material.h>
#include <Raytracing/Components/Material/raytracingMaterial.h>
#include <glad/glad.h>
#include <Core/ECS/registry.h>
#include <Components/Render/render.h>

extern bool dragging;

UICreation::MaterialComponentWindow::MaterialComponentWindow(int &currentEntity, Engine::Registry &registry)
    : ComponentWindow{"Material", currentEntity, registry}
{
}
    

void UICreation::MaterialComponentWindow::render() {
    bool hasOpenGLMaterial = m_registry.hasComponent<Engine::OpenGLMaterialComponent>(m_selectedEntity);
    bool hasRaytraceMaterial = m_registry.hasComponent<Engine::RaytracingMaterial>(m_selectedEntity);
    if (hasOpenGLMaterial || hasRaytraceMaterial) {
        ComponentWindow::render();
    }
}

void UICreation::MaterialComponentWindow::main() {
    if (auto material{m_registry.getComponent<Engine::OpenGLMaterialComponent>(m_currentEntity)}) {
        ImGui::Text("OpenGL Material");
        ImGui::SameLine();
        ImGui::Button("Drag##OpenGLMaterial");
        createImGuiComponentDragSource<Engine::OpenGLMaterialComponent>(material);
        if (!m_registry.hasComponent<Engine::RenderComponent>(m_currentEntity)) {
            ImGui::SameLine();
            if (ImGui::Button("x##OpenGL")) {
                m_registry.removeComponent<Engine::OpenGLMaterialComponent>(m_currentEntity);
            }
        }
        

        std::vector<Engine::MaterialUniformData> &materialsData{material->getMaterialData().second};

        for (Engine::MaterialUniformData &materialData : materialsData)
        {
            const char *name{std::get<0>(materialData).c_str()};
            unsigned int type{std::get<1>(materialData)};
            int offset{std::get<2>(materialData)};

            switch (type)
            {
            case GL_FLOAT:
                ImGui::DragFloat(name, material->getProperty<float>(offset));
                if (ImGui::IsItemEdited())
                {
                    material->update();
                    m_registry.updated<Engine::OpenGLMaterialComponent>(selectedEntity);
                }
                break;
            case GL_FLOAT_VEC4:
                ImGui::ColorEdit4(name, material->getProperty<float>(offset));
                if (ImGui::IsItemEdited())
                {
                    material->update();
                    m_registry.updated<Engine::OpenGLMaterialComponent>(selectedEntity);
                }
                break;
            case GL_FLOAT_VEC3:
                ImGui::ColorEdit3(name, material->getProperty<float>(offset));
                if (ImGui::IsItemEdited())
                {
                    material->update();
                    m_registry.updated<Engine::OpenGLMaterialComponent>(selectedEntity);
                }
                break;
            }
        }

        ImGui::Separator();
    }

    if (auto raytracingMaterial = m_registry.getComponent<Engine::RaytracingMaterial>(selectedEntity))
    {
        ImGui::Text("Raytracing Material");
        ImGui::SameLine();
        if (ImGui::Button("x##Raytrace")) {
            m_registry.removeComponent<Engine::RaytracingMaterial>(m_currentEntity);
        }

        ImGui::ColorEdit4("Color##Raytrace", raytracingMaterial->getColor().raw());
        bool isReflective = raytracingMaterial->isReflective();
        ImGui::Checkbox("Reflective##Raytrace", &isReflective);
        if (ImGui::IsItemClicked(0))
        {
            if (!isReflective)
            {
                raytracingMaterial->makeReflective();
            }
            else
            {
                raytracingMaterial->makeUnreflective();
            }
        }
    }
    else
    {
        if (ImGui::Button("Add Raytracing Material"))
        {
            m_registry.createComponent<Engine::RaytracingMaterial>(selectedEntity);
        }
    }
}