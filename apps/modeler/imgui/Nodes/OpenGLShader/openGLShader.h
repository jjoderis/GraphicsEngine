#ifndef APPS_MODELER_IMGUI_NODES_OPENGLSHADER
#define APPS_MODELER_IMGUI_NODES_OPENGLSHADER

#include "../helpers.h"
#include "../Templates/componentWindow.h"

namespace Engine
{
class OpenGLShaderComponent;
class OpenGLShader;
class Registry;
} // namespace Engine
namespace UICreation
{

class OpenGLShaderComponentWindow : public TemplatedComponentWindow<Engine::OpenGLShaderComponent> {

public:

  OpenGLShaderComponentWindow() = delete;
  OpenGLShaderComponentWindow(int &currentEntity, Engine::Registry &registry);

private:
  std::vector<const char *> m_shaderTypes{};
  std::vector<Engine::OpenGLShader> m_shaders{};
  int m_currentShader = 0;
  bool m_modalOpen{0};

  virtual void main() override;

  virtual void onComponentChange(std::shared_ptr<Engine::OpenGLShaderComponent> oldComponent) override;
  void loadShaders(const std::vector<Engine::OpenGLShader> &shaders);
  void loadShader(const Engine::OpenGLShader &shader);
  void drawShaderEditModal(unsigned int entity,
                         Engine::Registry &registry,
                         const std::shared_ptr<Engine::OpenGLShaderComponent> &shader);

};

}

#endif