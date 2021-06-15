#include "../helpers.h"
#include <OpenGL/Components/Material/material.h>
#include <glad/glad.h>

extern bool dragging;

template <>
void UICreation::createComponentNodeMain<Engine::OpenGLMaterialComponent>(
    std::shared_ptr<Engine::OpenGLMaterialComponent> material, Engine::Registry &registry)
{
    createImGuiComponentDragSource<Engine::OpenGLMaterialComponent>(dragging);

    std::vector<Engine::MaterialUniformData> &materialsData{std::get<1>(material->getMaterialData())};

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
}