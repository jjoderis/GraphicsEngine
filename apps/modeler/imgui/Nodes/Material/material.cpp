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
                            registry.updated<Engine::MaterialComponent>(selectedEntity);
                        }
                        break;
                    case GL_FLOAT_VEC4:
                        ImGui::ColorEdit4(name, material->getProperty<float>(offset));
                        if (ImGui::IsItemEdited())
                        {
                            registry.updated<Engine::MaterialComponent>(selectedEntity);
                        }
                        break;
                    case GL_FLOAT_VEC3:
                        ImGui::ColorEdit3(name, material->getProperty<float>(offset));
                        if (ImGui::IsItemEdited())
                        {
                            registry.updated<Engine::MaterialComponent>(selectedEntity);
                        }
                        break;
                    }
                }
            });
    }
}