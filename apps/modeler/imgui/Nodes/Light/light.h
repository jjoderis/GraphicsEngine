#ifndef APPS_MODELER_IMGUI_NODES_LIGHT
#define APPS_MODELER_IMGUI_NODES_LIGHT

#include "../helpers.h"

namespace Engine
{
class AmbientLightComponent;
class DirectionalLightComponent;
class PointLightComponent;
class SpotLightComponent;
class Registry;
} // namespace Engine
namespace UICreation
{

template <>
void createComponentNodeMain<Engine::AmbientLightComponent>(std::shared_ptr<Engine::AmbientLightComponent> light,
                                                            Engine::Registry &registry);

template <>
void createComponentNodeMain<Engine::DirectionalLightComponent>(
    std::shared_ptr<Engine::DirectionalLightComponent> light, Engine::Registry &registry);

template <>
void createComponentNodeMain<Engine::PointLightComponent>(std::shared_ptr<Engine::PointLightComponent> light,
                                                          Engine::Registry &registry);

template <>
void createComponentNodeMain<Engine::SpotLightComponent>(std::shared_ptr<Engine::SpotLightComponent> light,
                                                         Engine::Registry &registry);

} // namespace UICreation

#endif