#include "entity.h"

void drawEntityNode(unsigned int entity, Engine::Registry &registry)
{
    if (std::shared_ptr<Engine::TagComponent> tag = registry.getComponent<Engine::TagComponent>(entity))
    {
        bool isOpen = ImGui::TreeNode(tag->get().c_str());

        if (ImGui::IsItemClicked(2))
        {
            selectedEntity = entity;
            possible_component_current = 0;
        }
        UICreation::createImGuiComponentDropTarget<Engine::MaterialComponent>(entity, registry);
        UICreation::createImGuiComponentDropTarget<Engine::GeometryComponent>(entity, registry);
        UICreation::createImGuiComponentDropTarget<Engine::TransformComponent>(entity, registry);
        UICreation::createImGuiComponentDropTarget<Engine::OpenGLRenderComponent>(entity, registry);

        ImGui::SameLine();
        std::string id{"x##"};
        id.append(std::to_string(entity));
        if (ImGui::Button(id.c_str()))
        {
            registry.removeEntity(entity);

            if (entity == selectedEntity)
            {
                selectedEntity = -1;
            }
        }

        if (isOpen)
        {
            ImGui::TreePop();
        }
    }
}

char name[64];

void UICreation::drawEntitiesNode(Engine::Registry &registry)
{
    if (selectedEntity > -1)
    {
        const char *name = registry.getComponent<Engine::TagComponent>(selectedEntity)->get().c_str();
        ImGui::Text("Selection: %s", name);
    }
    else
    {
        ImGui::Text("Selection: None selected!");
    }

    std::list<unsigned int> entities{registry.getEntities()};

    if (ImGui::CollapsingHeader("Entities"))
    {
        for (unsigned int entity : entities)
        {
            drawEntityNode(entity, registry);
        }
        if (ImGui::Button("Add Entity"))
        {
            ImGui::OpenPopup("entity_add_popup");
        }
        if (ImGui::BeginPopup("entity_add_popup"))
        {
            ImGui::InputTextWithHint("##name_input", "Enter a name", name, IM_ARRAYSIZE(name));
            ImGui::SameLine();
            if (ImGui::Button("+"))
            {
                unsigned int newEntity = registry.addEntity();
                if (!std::strlen(name))
                {
                    registry.addComponent<Engine::TagComponent>(
                        newEntity, std::make_shared<Engine::TagComponent>("Unnamed Entity"));
                }
                else
                {
                    registry.addComponent<Engine::TagComponent>(newEntity,
                                                                std::make_shared<Engine::TagComponent>(name));
                }
                name[0] = '\0';
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("x"))
            {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }
}