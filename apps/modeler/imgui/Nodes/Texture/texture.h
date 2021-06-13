#ifndef APPS_MODELER_IMGUI_NODES_TEXTURE
#define APPS_MODELER_IMGUI_NODES_TEXTURE

#include "../helpers.h"

namespace Engine
{
class OpenGLTextureComponent;
class Registry;
} // namespace Engine
namespace UICreation
{

template <>
void createComponentNodeMain<Engine::OpenGLTextureComponent>(std::shared_ptr<Engine::OpenGLTextureComponent> texture,
                                                             Engine::Registry &registry);

}

#endif