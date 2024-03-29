#ifndef ENGINE_OPENGL_COMPONENTS_SHADER
#define ENGINE_OPENGL_COMPONENTS_SHADER

#include "../../Shader/shader.h"
#include "../Material/material.h"
#include <vector>

namespace Engine
{
class OpenGLShader;
class OpenGLProgram;

class OpenGLShaderComponent
{
public:
    OpenGLShaderComponent() = delete;
    OpenGLShaderComponent(std::vector<OpenGLShader> shaders);

    void updateShaders(std::vector<OpenGLShader> &newShaders);
    std::vector<OpenGLShader> getShaders();
    void useShader();

    ShaderMaterialData getMaterialProperties();

    int getLocation(const char* name);

private:
    Engine::OpenGLProgram m_program; // the shader program to use
    void setupUniforms();
};

} // namespace Engine

#endif