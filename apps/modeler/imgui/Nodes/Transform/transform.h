#ifndef APPS_MODELER_IMGUI_NODES_TRANSFORM
#define APPS_MODELER_IMGUI_NODES_TRANSFORM

#include "../helpers.h"

namespace Engine
{
class TransformComponent;
class Registry;
} // namespace Engine
namespace UICreation
{

template <>
void createComponentNodeMain<Engine::TransformComponent>(std::shared_ptr<Engine::TransformComponent> transform,
                                                         Engine::Registry &registry);

}

#endif