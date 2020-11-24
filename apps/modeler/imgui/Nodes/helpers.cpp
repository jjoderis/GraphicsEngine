#include "helpers.h"

bool dragging{1};

template <typename ComponentType>
void UICreation::createImGuiComponentDragSource() {
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

template void UICreation::createImGuiComponentDragSource<Engine::TransformComponent>();
template void UICreation::createImGuiComponentDragSource<Engine::OpenGLRenderComponent>();
template void UICreation::createImGuiComponentDragSource<Engine::MaterialComponent>();
template void UICreation::createImGuiComponentDragSource<Engine::GeometryComponent>();
template void UICreation::createImGuiComponentDragSource<Engine::CameraComponent>();

template <typename ComponentType>
void UICreation::createImGuiComponentDropTarget(unsigned int entity, Engine::Registry& registry) {
    if (ImGui::BeginDragDropTarget())
    {
        char dragDropType[256]{};
        sprintf(dragDropType, "Component_Drag_%u", Engine::type_index<ComponentType>::value());


        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(dragDropType))
        {
            registry.addComponent<ComponentType>(entity, registry.getComponent<ComponentType>(selectedEntity));
        }
        ImGui::EndDragDropTarget();
    }
}

template void UICreation::createImGuiComponentDropTarget<Engine::TransformComponent>(unsigned int entity, Engine::Registry& registry);
template void UICreation::createImGuiComponentDropTarget<Engine::OpenGLRenderComponent>(unsigned int entity, Engine::Registry& registry);
template void UICreation::createImGuiComponentDropTarget<Engine::MaterialComponent>(unsigned int entity, Engine::Registry& registry);
template void UICreation::createImGuiComponentDropTarget<Engine::GeometryComponent>(unsigned int entity, Engine::Registry& registry);
template void UICreation::createImGuiComponentDropTarget<Engine::CameraComponent>(unsigned int entity, Engine::Registry& registry);

template <typename ComponentType>
void UICreation::createComponentNodeOutline(const char* componentName, Engine::Registry& registry, ComponentType* component, std::function<void(void)> drawFunc) {
    // TODO: find out why right click is not always doing something, drag and drop seems to be buggy too (maybe somthing with the ids)
    if (ImGui::CollapsingHeader(componentName)) {
        char buff[64]{'\0'};
        sprintf(buff, "%s_remove_popup", componentName);
        if(ImGui::IsItemClicked(1)) {
            ImGui::OpenPopup(buff);
        }
        // dont draw a component after it was removed
        bool removed{false};
        if (ImGui::BeginPopup(buff))
        {
            sprintf(buff, "Remove %s", componentName);
            if (ImGui::Button(buff)) {
                registry.removeComponent<ComponentType>(selectedEntity);
                removed = true;
            }
            ImGui::EndPopup();
        }
        createImGuiComponentDragSource<ComponentType>();
        if (!removed) {
            drawFunc();
        }
    }
}

template void UICreation::createComponentNodeOutline<Engine::TransformComponent>(const char* componentName, Engine::Registry& registry, Engine::TransformComponent* component, std::function<void(void)> drawFunc);
template void UICreation::createComponentNodeOutline<Engine::OpenGLRenderComponent>(const char* componentName, Engine::Registry& registry, Engine::OpenGLRenderComponent* component, std::function<void(void)> drawFunc);
template void UICreation::createComponentNodeOutline<Engine::MaterialComponent>(const char* componentName, Engine::Registry& registry, Engine::MaterialComponent* component, std::function<void(void)> drawFunc);
template void UICreation::createComponentNodeOutline<Engine::GeometryComponent>(const char* componentName, Engine::Registry& registry, Engine::GeometryComponent* component, std::function<void(void)> drawFunc);
template void UICreation::createComponentNodeOutline<Engine::CameraComponent>(const char* componentName, Engine::Registry& registry, Engine::CameraComponent* component, std::function<void(void)> drawFunc);