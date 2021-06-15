#ifndef APPS_MODELER_IMGUI_NODES_HELPERS
#define APPS_MODELER_IMGUI_NODES_HELPERS

#include <Core/ECS/registry.h>
#include <imgui.h>
#include <memory>

extern int selectedEntity;
extern int possible_component_current;

namespace UICreation
{

template <typename ComponentType>
void createImGuiComponentDragSource(bool &dragging)
{
    ImGui::Button("Start Drag");
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        char dragDropType[256]{};
        sprintf(dragDropType, "Component_Drag_%u", Engine::type_index<ComponentType>::value());

        ImGui::SetDragDropPayload(dragDropType, &dragging, sizeof(bool));

        ImGui::Text("Assign Component");
        ImGui::EndDragDropSource();
    }
}

template <typename ComponentType>
void createImGuiComponentDropTarget(unsigned int entity, Engine::Registry &registry)
{
    if (ImGui::BeginDragDropTarget())
    {
        char dragDropType[256]{};
        sprintf(dragDropType, "Component_Drag_%u", Engine::type_index<ComponentType>::value());

        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(dragDropType))
        {
            registry.addComponent<ComponentType>(entity, registry.getComponent<ComponentType>(selectedEntity));
        }
        ImGui::EndDragDropTarget();
    }
}

template <typename ComponentType>
bool createComponentNodeStart(const char *componentName)
{
    return ImGui::Begin(componentName);
}

template <typename ComponentType>
bool makeRemovable(const char *componentName, Engine::Registry &registry)
{
    bool wasRemoved = false;
    char buff[64]{'\0'};
    sprintf(buff, "%s_remove_popup", componentName);
    if (ImGui::IsItemClicked(1))
    {
        ImGui::OpenPopup(buff);
    }
    // dont draw a component after it was removed
    if (ImGui::BeginPopup(buff))
    {
        sprintf(buff, "Remove %s", componentName);
        if (ImGui::Button(buff))
        {
            registry.removeComponent<ComponentType>(selectedEntity);
            wasRemoved = true;
        }
        ImGui::EndPopup();
    }

    return wasRemoved;
}

template <typename ComponentType>
bool createHeaderControls(const char *componentName, Engine::Registry &registry)
{
    return makeRemovable<ComponentType>(componentName, registry);
}

template <typename ComponentType>
void createComponentNodeMain(std::shared_ptr<ComponentType> component, Engine::Registry &registry)
{
    ImGui::Text("Implement Me");
}

inline void createComponentNodeEnd() { ImGui::End(); }

} // namespace UICreation

#endif