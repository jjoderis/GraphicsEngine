#include "../helpers.h"
#include <OpenGL/Components/Material/material.h>
#include <Raytracing/Components/Material/raytracingMaterial.h>
#include <glad/glad.h>

extern bool dragging;

template <>
void UICreation::createComponentNodeMain<Engine::OpenGLMaterialComponent>(
    std::shared_ptr<Engine::OpenGLMaterialComponent> material, Engine::Registry &registry)
{
    createImGuiComponentDragSource<Engine::OpenGLMaterialComponent>(dragging);

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
                registry.updated<Engine::OpenGLMaterialComponent>(selectedEntity);
            }
            break;
        case GL_FLOAT_VEC4:
            ImGui::ColorEdit4(name, material->getProperty<float>(offset));
            if (ImGui::IsItemEdited())
            {
                material->update();
                registry.updated<Engine::OpenGLMaterialComponent>(selectedEntity);
            }
            break;
        case GL_FLOAT_VEC3:
            ImGui::ColorEdit3(name, material->getProperty<float>(offset));
            if (ImGui::IsItemEdited())
            {
                material->update();
                registry.updated<Engine::OpenGLMaterialComponent>(selectedEntity);
            }
            break;
        }
    }

    if (auto raytracingMaterial = registry.getComponent<Engine::RaytracingMaterial>(selectedEntity))
    {
        ImGui::Separator();
        ImGui::Text("Raytracing Material");

        ImGui::ColorEdit4("Color", raytracingMaterial->getColor().raw());
        bool isReflective = raytracingMaterial->isReflective();
        ImGui::Checkbox("Reflective", &isReflective);
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
            registry.addComponent<Engine::RaytracingMaterial>(selectedEntity,
                                                              std::make_shared<Engine::RaytracingMaterial>());
        }
    }
}