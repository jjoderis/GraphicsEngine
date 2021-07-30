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
void createImGuiComponentDragSource(std::shared_ptr<ComponentType> component)
{
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        char dragDropType[256]{};
        sprintf(dragDropType, "Component_Drag_%u", Engine::type_index<ComponentType>::value());

        ImGui::SetDragDropPayload(dragDropType, &component, sizeof(std::shared_ptr<ComponentType>));

        ImGui::Text("Assign Component");
        ImGui::EndDragDropSource();
    }
}

template <typename ComponentType>
std::shared_ptr<ComponentType> createImGuiComponentDropTarget()
{
    std::shared_ptr<ComponentType> component;
    if (ImGui::BeginDragDropTarget())
    {
        char dragDropType[256]{};
        sprintf(dragDropType, "Component_Drag_%u", Engine::type_index<ComponentType>::value());

        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload(dragDropType))
        {
            component = *(std::shared_ptr<ComponentType>*)payload->Data;
        }
        ImGui::EndDragDropTarget();
    }
    return component;
}

} // namespace UICreation

#endif