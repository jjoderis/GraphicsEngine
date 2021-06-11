#ifndef APPS_MODELER_IMGUI_NODES_OPENGLSHADER
#define APPS_MODELER_IMGUI_NODES_OPENGLSHADER

#include "../helpers.h"

namespace Engine
{
class OpenGLShaderComponent;
class Registry;
} // namespace Engine
namespace UICreation
{

template <>
void createComponentNodeMain<Engine::OpenGLShaderComponent>(std::shared_ptr<Engine::OpenGLShaderComponent> transform,
                                                            Engine::Registry &registry);

}

#endif