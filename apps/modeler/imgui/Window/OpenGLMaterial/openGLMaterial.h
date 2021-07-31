#ifndef APPS_MODELER_IMGUI_NODES_MATERIAL
#define APPS_MODELER_IMGUI_NODES_MATERIAL

#include "../Templates/componentWindow.h"
#include <OpenGL/Util/textureIndex.h>
#include <Components/Shader/shader.h>

namespace Engine
{
class Registry;
} // namespace Engine
namespace UICreation
{

class MaterialComponentWindow : public ComponentWindow {

public:

  MaterialComponentWindow() = delete;
  MaterialComponentWindow(int &currentEntity, Engine::Registry &registry, Engine::Util::OpenGLTextureIndex &textureIndex);

  virtual void render() override;

private:
  Engine::Util::OpenGLTextureIndex& m_textureIndex;

  std::shared_ptr<Engine::OpenGLShaderComponent> m_shader;
  std::vector<const char *> m_shaderTypes{};
  std::vector<Engine::OpenGLShader> m_shaders{};
  int m_currentShader = 0;
  bool m_modalOpen{0};

  virtual void main() override;

  virtual void onEntityChange(int oldEntity) override;

  virtual void checkUpdates() override;

  void checkShaderChange();

  void loadShaders(const std::vector<Engine::OpenGLShader> &shaders);
  void loadShader(const Engine::OpenGLShader &shader);
  void drawShaderEditModal(unsigned int entity,
                         Engine::Registry &registry,
                         const std::shared_ptr<Engine::OpenGLShaderComponent> &shader);

};

}

#endif