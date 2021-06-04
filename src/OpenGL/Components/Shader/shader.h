#ifndef ENGINE_OPENGL_COMPONENTS_SHADER
#define ENGINE_OPENGL_COMPONENTS_SHADER

#include "../../../Core/Components/Material/material.h"
#include "../../Shader/shader.h"
#include <vector>

namespace Engine
{

class OpenGLShaderComponent
{
public:
    OpenGLShaderComponent() = delete;
    OpenGLShaderComponent(std::vector<OpenGLShader> shaders);

    void updateShaders(std::vector<OpenGLShader> &newShaders);
    std::vector<OpenGLShader> getShaders();
    void useShader();

    ShaderMaterialData getMaterialProperties();

private:
    Engine::OpenGLProgram m_program; // the shader program to use
    void setupUniforms();
};

} // namespace Engine

#endif