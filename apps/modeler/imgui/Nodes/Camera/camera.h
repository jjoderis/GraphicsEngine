#ifndef APPS_MODELER_IMGUI_NODES_CAMERA
#define APPS_MODELER_IMGUI_NODES_CAMERA

#include "../helpers.h"

namespace Engine
{
class CameraComponent;
class Registry;
} // namespace Engine
namespace UICreation
{

template <>
void createComponentNodeMain<Engine::CameraComponent>(std::shared_ptr<Engine::CameraComponent> camera,
                                                      Engine::Registry &registry);

}

#endif