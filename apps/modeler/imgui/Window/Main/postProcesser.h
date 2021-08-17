#ifndef APPS_MODELER_IMGUI_WINDOW_MAIN_POSTPROCESSER
#define APPS_MODELER_IMGUI_WINDOW_MAIN_POSTPROCESSER

#include <OpenGL/Framebuffer/framebuffer.h>
#include <OpenGL/Shader/shader.h>
#include <OpenGL/Systems/CameraTracker/cameraTracker.h>

namespace Engine
{
class Registry;
}

namespace ModelerUtil
{

class PostProcesser
{
public:
    PostProcesser() = delete;

    PostProcesser(Engine::Registry &registry, int &selectedEntity);

    void resize(int width, int height);

    void postProcess(unsigned int renderedScene);

    Engine::OpenGLFramebuffer &getFramebuffer();

private:
    // Trying out kernels based on https://learnopengl.com/Advanced-OpenGL/Framebuffers
    Engine::Registry &m_registry;
    int &m_selectedEntity;

    const char *selectedVertexShader = "#version 420 core\n"
                                       "layout (location = 0) in vec3 pos;\n"
                                       "uniform Transform{"
                                       "    mat4 modelMatrix;"
                                       "    mat4 normalMatrix;"
                                       " };"

                                       " uniform Camera{"
                                       "     mat4 viewMatrix;"
                                       "     mat4 viewMatrixInverse;"
                                       "     mat4 projectionMatrix;"
                                       " };"
                                       "void main () {\n"
                                       "  gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(pos, 1);\n"
                                       "}\n";

    const char *selectedFragmentShader = "#version 420 core\n"
                                         "out vec4 FragColor;\n"
                                         "uniform int isSelected;"
                                         "void main()\n"
                                         "{\n"
                                         "  if (isSelected == 1) {"
                                         "    FragColor = vec4(1.0, 0.0, 0.0, 1.0);"
                                         "  } else {"
                                         "    FragColor = vec4(0.0, 1.0, 0.0, 1.0);"
                                         "  }"
                                         "}";

    Engine::OpenGLProgram m_selectedProgram{
        {{GL_VERTEX_SHADER, selectedVertexShader}, {GL_FRAGMENT_SHADER, selectedFragmentShader}}};
    Engine::OpenGLFramebuffer m_selectedFramebuffer{};
    unsigned int m_activeCameraUBO{};
    int m_cameraIndex{0};
    int m_transformIndex{1};
    int m_isSelectedIndex{-1};
    Engine::Systems::OpenGLCameraTracker m_cameraTracker;

    const char *vertexShader = "#version 420 core\n"
                               "layout (location = 0) in vec3 pos;\n"
                               "layout (location = 1) in vec2 tex;\n"
                               "out vec2 texCoord;"
                               "void main () {\n"
                               "  texCoord = tex;"
                               "  gl_Position = vec4(pos, 1);\n"
                               "}\n";

    const char *fragmentShader = "#version 420 core\n"
                                 "out vec4 FragColor;\n"
                                 "in vec2 texCoord;\n"
                                 "uniform int selectedEntity;"
                                 "layout (binding = 0) uniform sampler2D sceneTexture;\n"
                                 "layout (binding = 1) uniform sampler2D selectedTexture;\n"
                                 "const float offset = 1.0 / 300.0;\n"

                                 "void main()\n"
                                 "{\n"
                                 "  vec2 offsets[9] = vec2[](\n"
                                 "  vec2(-offset,  offset), // top-left\n"
                                 "  vec2( 0.0f,    offset), // top-center\n"
                                 "  vec2( offset,  offset), // top-right\n"
                                 "  vec2(-offset,  0.0f),   // center-left\n"
                                 "  vec2( 0.0f,    0.0f),   // center-center\n"
                                 "  vec2( offset,  0.0f),   // center-right\n"
                                 "  vec2(-offset, -offset), // bottom-left\n"
                                 "  vec2( 0.0f,   -offset), // bottom-center\n"
                                 "  vec2( offset, -offset)  // bottom-right\n"
                                 "  );\n"

                                 "  float kernel[9] = float[](\n"
                                 "    1, 1, 1,\n"
                                 "    1, -8, 1,\n"
                                 "    1, 1, 1 \n"
                                 "  );"

                                 "  vec3 preCol[9];\n"
                                 "  for(int i = 0; i < 9; i++)\n"
                                 "  {\n"
                                 "    preCol[i] = vec3(texture(selectedTexture, texCoord.st + offsets[i]));\n"
                                 "  }\n"
                                 "  vec3 col = vec3(0.0);\n"
                                 "  for(int i = 0; i < 9; i++) {\n"
                                 "      col += preCol[i] * kernel[i];\n"
                                 "  }"

                                 "  if (col.x > 0.001 || col.y > 0.001 || col.z > 0.001) {"
                                 "    FragColor = vec4(col, 1.0);"
                                 "  } else {"
                                 "    FragColor = texture(sceneTexture, texCoord);"
                                 "  }"
                                 "}";

    Engine::OpenGLFramebuffer m_frameBuffer{};
    unsigned int m_VBO{0};
    unsigned int m_EBO{0};
    unsigned int m_VAO{0};
    Engine::OpenGLProgram m_program{{{GL_VERTEX_SHADER, vertexShader}, {GL_FRAGMENT_SHADER, fragmentShader}}};
};

}; // namespace ModelerUtil

#endif