#ifndef APPS_MODELER_IMGUI_NODES_HELPERS
#define APPS_MODELER_IMGUI_NODES_HELPERS

#include <Core/ECS/registry.h>
#include <imgui.h>
#include <memory>
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <functional>
#include <imgui_internal.h>

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

template <typename PayloadType>
PayloadType *createImGuiHighlightedDropTarget(const char *payloadType,
                                              std::function<bool(const PayloadType &)> canDropFunction)
{
    auto &context{*ImGui::GetCurrentContext()};
    auto payload{context.DragDropPayload};
    if (context.DragDropActive && payload.IsDataType(payloadType))
    {
        auto payloadData{(PayloadType *)payload.Data};

        if (canDropFunction(*payloadData))
        {
            if (ImGui::BeginDragDropTarget())
            {
                if (ImGui::AcceptDragDropPayload(payloadType, ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
                {
                    ImGui::EndDragDropTarget();
                    return payloadData;
                }
                ImGui::EndDragDropTarget();
            }

            ImRect r{ImGui::GetItemRectMin(), ImGui::GetItemRectMax()};
            auto window{ImGui::GetCurrentWindow()};
            r.Expand(3.5f);
            // if (!window->ClipRect.Contains(r))
            // {
            window->DrawList->PushClipRect(r.Min - ImVec2(1, 1), r.Max + ImVec2(1, 1));
            window->DrawList->AddRect(r.Min, r.Max, ImGui::GetColorU32(ImGuiCol_DragDropTarget), 0.0f, ~0, 2.0f);
            window->DrawList->PopClipRect();
            // }
        }
    };

    return nullptr;
}

template <typename ComponentType>
std::shared_ptr<ComponentType> createImGuiComponentDropTarget()
{
    char dragDropType[256]{};
    sprintf(dragDropType, "Component_Drag_%u", Engine::type_index<ComponentType>::value());
    auto component{createImGuiHighlightedDropTarget<std::shared_ptr<ComponentType>>(
        dragDropType, [](const std::shared_ptr<ComponentType> &component) { return true; })};

    return *component;
}

} // namespace UICreation

#endif