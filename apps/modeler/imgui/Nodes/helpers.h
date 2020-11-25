#ifndef APPS_MODELER_IMGUI_NODES_HELPERS
#define APPS_MODELER_IMGUI_NODES_HELPERS

#include <imgui.h>
#include <Core/ECS/registry.h>
#include <Core/Components/Transform/transform.h>
#include <OpenGL/Components/Render/render.h>
#include <Core/Components/Material/material.h>
#include <Core/Components/Geometry/geometry.h>
#include <Core/Components/Camera/camera.h>
#include <Core/Components/Tag/tag.h>

extern int selectedEntity;
extern int possible_component_current;
extern std::string errorMessage;

// keep track of the selected entities components without keeping them alive (weak_ptr instead of shared_ptr)
extern std::weak_ptr<Engine::TagComponent> selectedTag;
extern std::weak_ptr<Engine::TransformComponent> selectedTransform;
extern std::weak_ptr<Engine::OpenGLRenderComponent> selectedRender;
extern std::weak_ptr<Engine::MaterialComponent> selectedMaterial;
extern std::weak_ptr<Engine::GeometryComponent> selectedGeometry;
extern std::weak_ptr<Engine::CameraComponent> selectedCamera;

namespace UICreation {

template <typename ComponentType>
void createImGuiComponentDragSource();

template <typename ComponentType>
void createImGuiComponentDropTarget(unsigned int entity, Engine::Registry& registry);

template <typename ComponentType>
void createComponentNodeOutline(const char* componentName, Engine::Registry& registry, ComponentType* component, std::function<void(void)> drawFunc);

void drawErrorModal(std::string& errorMessage);

}

#endif