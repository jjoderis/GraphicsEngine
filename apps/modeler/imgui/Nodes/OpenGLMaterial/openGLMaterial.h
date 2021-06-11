#ifndef APPS_MODELER_IMGUI_NODES_MATERIAL
#define APPS_MODELER_IMGUI_NODES_MATERIAL

#include "../helpers.h"

namespace Engine
{
class OpenGLMaterialComponent;
class Registry;
} // namespace Engine
namespace UICreation
{

template <>
void createComponentNodeMain<Engine::OpenGLMaterialComponent>(std::shared_ptr<Engine::OpenGLMaterialComponent> material,
                                                              Engine::Registry &registry);

}

#endif