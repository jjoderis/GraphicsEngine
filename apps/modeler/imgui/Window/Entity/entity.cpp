#include "entity.h"

#include "../../Util/objectLoader.h"
#include "../helpers.h"
#include <Core/Components/Camera/camera.h>
#include <Core/Components/Geometry/geometry.h>
#include <Core/Components/Hierarchy/hierarchy.h>
#include <Core/Components/Light/light.h>
#include <Core/Components/Tag/tag.h>
#include <Core/Components/Transform/transform.h>
#include <Core/ECS/registry.h>
#include <OpenGL/Components/Material/material.h>
#include <OpenGL/Components/Shader/shader.h>
#include <OpenGL/Components/Texture/texture.h>
#include <OpenGL/Util/textureIndex.h>
#include <cstring>
#include <imgui.h>
#include <iostream>

extern Engine::Util::OpenGLTextureIndex textureIndex;

namespace fs = std::filesystem;

bool isCyclic(int entity, int currEntity, Engine::Registry &registry)
{
    if (currEntity == -1)
    {
        return false;
    }

    if (entity == currEntity)
    {
        return true;
    }

    auto hierarchy{registry.getComponent<Engine::HierarchyComponent>(currEntity)};

    if (!hierarchy)
    {
        return false;
    }

    return isCyclic(entity, hierarchy->getParent(), registry);
};

void createHierarchyDragAndDrop(int entity, Engine::Registry &registry)
{
    // create logic for when an entity is started to be dragged
    if (entity > -1 && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        // carry the entity id as payload
        ImGui::SetDragDropPayload("hierarchy_drag", &entity, sizeof(int));

        std::shared_ptr<Engine::TagComponent> tag = registry.getComponent<Engine::TagComponent>(entity);

        ImGui::Text("Assign %s to new parent!", tag->get().c_str());

        ImGui::EndDragDropSource();
    }

    if (auto dragEntity = UICreation::createImGuiHighlightedDropTarget<int>(
            "hierarchy_drag",
            [&registry, entity](const int &dragEntity) { return !isCyclic(dragEntity, entity, registry); }))
    {
        std::shared_ptr<Engine::HierarchyComponent> hierarchy =
            registry.getComponent<Engine::HierarchyComponent>(*dragEntity);
        if (!hierarchy)
        {
            hierarchy = registry.createComponent<Engine::HierarchyComponent>(*dragEntity);
        }
        // see if we assigned a new parent and change hierarchy component if we did
        if (entity != hierarchy->getParent())
        {
            hierarchy->setParent(entity);
            registry.updated<Engine::HierarchyComponent>(*dragEntity);
        }
    }
}

void removeWithChildren(Engine::Registry &registry, unsigned int entity)
{
    if (entity == selectedEntity)
    {
        selectedEntity = -1;
    }

    if (auto hierarchy = registry.getComponent<Engine::HierarchyComponent>(entity))
    {
        auto &children = hierarchy->getChildren();
        for (int i = children.size() - 1; i >= 0; --i)
        {
            removeWithChildren(registry, children[i]);
        }
    }

    registry.removeEntity(entity);
}

template <typename ComponentType>
void createComponentDrop(unsigned int entity, Engine::Registry &registry)
{
    char dragDropType[256]{};
    sprintf(dragDropType, "Component_Drag_%u", Engine::type_index<ComponentType>::value());
    if (auto component = UICreation::createImGuiHighlightedDropTarget<std::shared_ptr<ComponentType>>(
            dragDropType,
            [&registry, entity](const std::shared_ptr<ComponentType> &component)
            {
                auto entityComponent{registry.getComponent<ComponentType>(entity)};

                return entityComponent != component;
            }))
    {
        registry.addComponent<ComponentType>(entity, *component);
    }
}

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
        createHierarchyDragAndDrop(entity, registry);
        createComponentDrop<Engine::OpenGLMaterialComponent>(entity, registry);
        createComponentDrop<Engine::GeometryComponent>(entity, registry);
        createComponentDrop<Engine::OpenGLShaderComponent>(entity, registry);
        createComponentDrop<Engine::OpenGLTextureComponent>(entity, registry);

        if (auto path = UICreation::createImGuiHighlightedDropTarget<fs::path>(
                "system_path_payload",
                [](const fs::path &path) { return fs::is_regular_file(path) && (path.extension() == ".off"); }))
        {
            if (path->extension() == ".off")
            {
                registry.addComponent<Engine::GeometryComponent>(entity, Engine::loadOffFile(*path));
            }
        }

        ImGui::SameLine();
        std::string id{"x##"};
        id.append(std::to_string(entity));
        if (ImGui::Button(id.c_str()))
        {
            if (ImGui::GetIO().KeyShift)
            {
                // remove recursively if shift is pressed
                removeWithChildren(registry, entity);
            }
            else
            {
                registry.removeEntity(entity);

                if (entity == selectedEntity)
                {
                    selectedEntity = -1;
                }
            }
        }

        std::shared_ptr<Engine::HierarchyComponent> hierarchy =
            registry.getComponent<Engine::HierarchyComponent>(entity);
        if (isOpen && hierarchy)
        {
            for (unsigned int child : hierarchy->getChildren())
            {
                drawEntityNode(child, registry);
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
        createHierarchyDragAndDrop(-1, registry);

        for (unsigned int entity : entities)
        {
            std::shared_ptr<Engine::HierarchyComponent> hierarchy =
                registry.getComponent<Engine::HierarchyComponent>(entity);
            // only show top level entities in root of tree
            if (!hierarchy || hierarchy->getParent() < 0)
            {
                drawEntityNode(entity, registry);
            }
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
                    registry.createComponent<Engine::TagComponent>(newEntity, "Unnamed Entity");
                }
                else
                {
                    registry.createComponent<Engine::TagComponent>(newEntity, name);
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