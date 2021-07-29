#ifndef APPS_MODELER_IMGUI_NODES_TEXTURE
#define APPS_MODELER_IMGUI_NODES_TEXTURE

#include "../helpers.h"
#include "../Templates/componentWindow.h"
#include <OpenGL/Util/textureIndex.h>
namespace Engine
{
class OpenGLTextureComponent;
class Registry;
} // namespace Engine
namespace UICreation
{

class TextureComponentWindow : public TemplatedComponentWindow<Engine::OpenGLTextureComponent> {

public:

  TextureComponentWindow() = delete;
  TextureComponentWindow(int &currentEntity, Engine::Registry &registry, Engine::Util::OpenGLTextureIndex &textureIndex);

private:
  Engine::Util::OpenGLTextureIndex& m_textureIndex;

  virtual void main() override;

};

}

#endif