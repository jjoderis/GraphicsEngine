#ifndef APPS_MODELER_IMGUI_NODES_HELPERS
#define APPS_MODELER_IMGUI_NODES_HELPERS

#include <Core/Components/Camera/camera.h>
#include <Core/Components/Geometry/geometry.h>
#include <Core/Components/Light/light.h>
#include <Core/Components/Material/material.h>
#include <Core/Components/Tag/tag.h>
#include <Core/Components/Transform/transform.h>
#include <Core/ECS/registry.h>
#include <OpenGL/Components/Render/render.h>
#include <imgui.h>

extern int selectedEntity;
extern int possible_component_current;

namespace UICreation
{

template <typename ComponentType>
void createImGuiComponentDragSource();

template <typename ComponentType>
void createImGuiComponentDropTarget(unsigned int entity, Engine::Registry &registry);

template <typename ComponentType>
void createComponentNodeOutline(const char *componentName,
                                Engine::Registry &registry,
                                ComponentType *component,
                                std::function<void(void)> drawFunc);

} // namespace UICreation

#endif