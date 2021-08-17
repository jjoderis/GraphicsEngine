#ifndef APPS_MODELER_IMGUI_NODES_MATERIAL
#define APPS_MODELER_IMGUI_NODES_MATERIAL

#include "../Templates/componentWindow.h"
#include "./shaderWindow.h"
#include <Components/Shader/shader.h>
#include <OpenGL/Util/textureIndex.h>

namespace Engine
{
class Registry;
} // namespace Engine
namespace UICreation
{

class MaterialComponentWindow : public ComponentWindow
{

public:
    MaterialComponentWindow() = delete;
    MaterialComponentWindow(int &currentEntity,
                            Engine::Registry &registry,
                            Engine::Util::OpenGLTextureIndex &textureIndex);

    virtual void render() override;

private:
    Engine::Util::OpenGLTextureIndex &m_textureIndex;
    OpenGLShaderCodeWindow m_shaderWindow;

    virtual void main() override;
};

} // namespace UICreation

#endif