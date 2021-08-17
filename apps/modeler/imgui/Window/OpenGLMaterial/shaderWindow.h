#ifndef APPS_MODELER_IMGUI_WINDOW_MATERIAL_SHADER_CODE
#define APPS_MODELER_IMGUI_WINDOW_MATERIAL_SHADER_CODE

#include "../Templates/componentWindow.h"

namespace Engine
{
class OpenGLShaderComponent;
class OpenGLShader;
class Registry;
} // namespace Engine
namespace UICreation
{

class OpenGLShaderCodeWindow : public TemplatedComponentWindow<Engine::OpenGLShaderComponent>
{

public:
    OpenGLShaderCodeWindow() = delete;
    OpenGLShaderCodeWindow(int &currentEntity, Engine::Registry &registry);

    void open();

    std::shared_ptr<Engine::OpenGLShaderComponent> getShader();

    virtual void render();

private:
    std::vector<const char *> m_shaderTypes{};
    std::vector<Engine::OpenGLShader> m_shaders{};
    int m_currentShader = 0;

    virtual void main() override;
    virtual void onComponentChange(std::shared_ptr<Engine::OpenGLShaderComponent> oldShader);

    void loadShaders(const std::vector<Engine::OpenGLShader> &shaders);
    void loadShader(const Engine::OpenGLShader &shader);
    void drawShaderEditModal(unsigned int entity,
                             Engine::Registry &registry,
                             const std::shared_ptr<Engine::OpenGLShaderComponent> &shader);
};

} // namespace UICreation

#endif