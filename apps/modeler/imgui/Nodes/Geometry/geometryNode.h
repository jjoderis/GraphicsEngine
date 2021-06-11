#ifndef APPS_MODELER_IMGUI_NODES_GEOMETRY
#define APPS_MODELER_IMGUI_NODES_GEOMETRY

#include "../helpers.h"

namespace Engine
{
class GeometryComponent;
class Registry;
} // namespace Engine
namespace UICreation
{

template <>
void createComponentNodeMain<Engine::GeometryComponent>(std::shared_ptr<Engine::GeometryComponent> geometry,
                                                        Engine::Registry &registry);

}

#endif