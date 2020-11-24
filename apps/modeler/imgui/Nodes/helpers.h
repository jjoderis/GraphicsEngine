#ifndef APPS_MODELER_IMGUI_NODES_HELPERS
#define APPS_MODELER_IMGUI_NODES_HELPERS

#include <imgui.h>
#include <Core/ECS/registry.h>
#include <Core/Components/Transform/transform.h>
#include <OpenGL/Components/Render/render.h>
#include <Core/Components/Material/material.h>
#include <Core/Components/Geometry/geometry.h>
#include <Core/Components/Camera/camera.h>

extern int selectedEntity;

namespace UICreation {

template <typename ComponentType>
void createImGuiComponentDragSource(ComponentType* component);

template <typename ComponentType>
void createImGuiComponentDropTarget(unsigned int entity, Engine::Registry& registry);

template <typename ComponentType>
void createComponentNodeOutline(const char* componentName, Engine::Registry& registry, ComponentType* component, std::function<void(void)> drawFunc);

}

#endif