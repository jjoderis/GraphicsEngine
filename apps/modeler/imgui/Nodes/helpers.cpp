#include "helpers.h"

bool dragging{1};

template <typename ComponentType>
void UICreation::createImGuiComponentDragSource()
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

// clang-format off
template void UICreation::createImGuiComponentDragSource<Engine::TransformComponent>();
template void UICreation::createImGuiComponentDragSource<Engine::OpenGLShaderComponent>();
template void UICreation::createImGuiComponentDragSource<Engine::OpenGLMaterialComponent>();
template void UICreation::createImGuiComponentDragSource<Engine::GeometryComponent>();
template void UICreation::createImGuiComponentDragSource<Engine::CameraComponent>();
template void UICreation::createImGuiComponentDragSource<Engine::AmbientLightComponent>();
template void UICreation::createImGuiComponentDragSource<Engine::DirectionalLightComponent>();
template void UICreation::createImGuiComponentDragSource<Engine::PointLightComponent>();
template void UICreation::createImGuiComponentDragSource<Engine::SpotLightComponent>();
// clang-format on

template <typename ComponentType>
void UICreation::createImGuiComponentDropTarget(unsigned int entity, Engine::Registry &registry)
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

// clang-format off
template void UICreation::createImGuiComponentDropTarget<Engine::TransformComponent>(unsigned int entity, Engine::Registry& registry);
template void UICreation::createImGuiComponentDropTarget<Engine::OpenGLShaderComponent>(unsigned int entity, Engine::Registry& registry);
template void UICreation::createImGuiComponentDropTarget<Engine::OpenGLMaterialComponent>(unsigned int entity, Engine::Registry& registry);
template void UICreation::createImGuiComponentDropTarget<Engine::GeometryComponent>(unsigned int entity, Engine::Registry& registry);
template void UICreation::createImGuiComponentDropTarget<Engine::CameraComponent>(unsigned int entity, Engine::Registry& registry);
template void UICreation::createImGuiComponentDropTarget<Engine::AmbientLightComponent>(unsigned int entity, Engine::Registry& registry);
template void UICreation::createImGuiComponentDropTarget<Engine::DirectionalLightComponent>(unsigned int entity, Engine::Registry& registry);
template void UICreation::createImGuiComponentDropTarget<Engine::PointLightComponent>(unsigned int entity, Engine::Registry& registry);
template void UICreation::createImGuiComponentDropTarget<Engine::SpotLightComponent>(unsigned int entity, Engine::Registry& registry);
// clang-format on

template <typename ComponentType>
void UICreation::createComponentNodeOutline(const char *componentName,
                                            Engine::Registry &registry,
                                            ComponentType *component,
                                            std::function<void(void)> drawFunc)
{
    if (ImGui::Begin(componentName))
    {
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
            }
            ImGui::EndPopup();
        }
        createImGuiComponentDragSource<ComponentType>();
        drawFunc();
    }
    ImGui::End();
}

// clang-format off
template void UICreation::createComponentNodeOutline<Engine::TransformComponent>(const char* componentName, Engine::Registry& registry, Engine::TransformComponent* component, std::function<void(void)> drawFunc);
template void UICreation::createComponentNodeOutline<Engine::OpenGLShaderComponent>(const char* componentName, Engine::Registry& registry, Engine::OpenGLShaderComponent* component, std::function<void(void)> drawFunc);
template void UICreation::createComponentNodeOutline<Engine::OpenGLMaterialComponent>(const char* componentName, Engine::Registry& registry, Engine::OpenGLMaterialComponent* component, std::function<void(void)> drawFunc);
template void UICreation::createComponentNodeOutline<Engine::GeometryComponent>(const char* componentName, Engine::Registry& registry, Engine::GeometryComponent* component, std::function<void(void)> drawFunc);
template void UICreation::createComponentNodeOutline<Engine::CameraComponent>(const char* componentName, Engine::Registry& registry, Engine::CameraComponent* component, std::function<void(void)> drawFunc);
template void UICreation::createComponentNodeOutline<Engine::AmbientLightComponent>(const char* componentName, Engine::Registry& registry, Engine::AmbientLightComponent* component, std::function<void(void)> drawFunc);
template void UICreation::createComponentNodeOutline<Engine::DirectionalLightComponent>(const char* componentName, Engine::Registry& registry, Engine::DirectionalLightComponent* component, std::function<void(void)> drawFunc);
template void UICreation::createComponentNodeOutline<Engine::PointLightComponent>(const char* componentName, Engine::Registry& registry, Engine::PointLightComponent* component, std::function<void(void)> drawFunc);
template void UICreation::createComponentNodeOutline<Engine::SpotLightComponent>(const char* componentName, Engine::Registry& registry, Engine::SpotLightComponent* component, std::function<void(void)> drawFunc);
// clang-format on