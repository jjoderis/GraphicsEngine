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
#include <cstring>
#include <imgui.h>
#include <iostream>
#include <misc/cpp/imgui_stdlib.h>

namespace fs = std::filesystem;

UICreation::EntityWindow::EntityWindow(Engine::Registry &registry) : m_registry{registry} {}

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

void UICreation::EntityWindow::drawEntityNode(unsigned int entity)
{
    if (std::shared_ptr<Engine::TagComponent> tag = m_registry.getComponent<Engine::TagComponent>(entity))
    {
        if (m_changeNameEntity == entity)
        {
            if (ImGui::InputText("Edit Name##Entity", &m_currentName))
            {
                ImGui::GetIO().WantCaptureKeyboard = true;
            }

            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
            {
                tag->set(m_currentName);
                m_currentName.clear();
                m_changeNameEntity = -1;
            }

            if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
            {
                m_currentName.clear();
                m_changeNameEntity = -1;
            }

            return;
        }

        auto treeNodeFlags{ImGuiTreeNodeFlags_None};

        if (entity == selectedEntity)
        {
            treeNodeFlags = ImGuiTreeNodeFlags_Selected;
        }

        bool isOpen = ImGui::TreeNodeEx(tag->get().c_str(), treeNodeFlags);

        if (ImGui::IsItemClicked(0) && ImGui::IsMouseDoubleClicked(0))
        {
            m_currentName.clear();
            m_changeNameEntity = entity;
        }

        if (ImGui::IsItemClicked(2))
        {
            selectedEntity = entity;
            possible_component_current = 0;
        }
        createHierarchyDragAndDrop(entity, m_registry);
        createComponentDrop<Engine::OpenGLMaterialComponent>(entity, m_registry);
        createComponentDrop<Engine::GeometryComponent>(entity, m_registry);
        createComponentDrop<Engine::OpenGLShaderComponent>(entity, m_registry);
        createComponentDrop<Engine::OpenGLTextureComponent>(entity, m_registry);

        if (auto path = UICreation::createImGuiHighlightedDropTarget<fs::path>(
                "system_path_payload",
                [](const fs::path &path) { return fs::is_regular_file(path) && (path.extension() == ".off"); }))
        {
            if (path->extension() == ".off")
            {
                m_registry.addComponent<Engine::GeometryComponent>(entity, Engine::loadOffFile(*path));
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
                removeWithChildren(m_registry, entity);
            }
            else
            {
                m_registry.removeEntity(entity);

                if (entity == selectedEntity)
                {
                    selectedEntity = -1;
                }
            }
        }

        std::shared_ptr<Engine::HierarchyComponent> hierarchy =
            m_registry.getComponent<Engine::HierarchyComponent>(entity);
        if (isOpen && hierarchy)
        {
            for (unsigned int child : hierarchy->getChildren())
            {
                drawEntityNode(child);
            }
        }

        if (isOpen)
        {
            ImGui::TreePop();
        }
    }
}

void UICreation::EntityWindow::render()
{
    if (selectedEntity > -1)
    {
        const char *name = m_registry.getComponent<Engine::TagComponent>(selectedEntity)->get().c_str();
        ImGui::Text("Selection: %s", name);
    }
    else
    {
        ImGui::Text("Selection: None selected!");
    }

    std::list<unsigned int> entities{m_registry.getEntities()};

    if (ImGui::CollapsingHeader("Entities"))
    {
        createHierarchyDragAndDrop(-1, m_registry);

        for (unsigned int entity : entities)
        {
            std::shared_ptr<Engine::HierarchyComponent> hierarchy =
                m_registry.getComponent<Engine::HierarchyComponent>(entity);
            // only show top level entities in root of tree
            if (!hierarchy || hierarchy->getParent() < 0)
            {
                drawEntityNode(entity);
            }
        }
        if (ImGui::Button("Add Entity"))
        {
            ImGui::OpenPopup("entity_add_popup");
        }
        if (ImGui::BeginPopup("entity_add_popup"))
        {
            ImGui::InputTextWithHint("##name_input", "Enter a name", &m_currentName);
            ImGui::SameLine();
            if (ImGui::Button("+"))
            {
                unsigned int newEntity = m_registry.addEntity();
                if (!m_currentName.size())
                {
                    m_registry.createComponent<Engine::TagComponent>(newEntity, "Unnamed Entity");
                }
                else
                {
                    m_registry.createComponent<Engine::TagComponent>(newEntity, m_currentName);
                }
                m_currentName.clear();
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