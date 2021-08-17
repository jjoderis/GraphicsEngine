#ifndef APPS_MODELER_IMGUI_WINDOW_MAIN_POSTPROCESSER
#define APPS_MODELER_IMGUI_WINDOW_MAIN_POSTPROCESSER

#include <OpenGL/Framebuffer/framebuffer.h>
#include <OpenGL/Shader/shader.h>

namespace ModelerUtil
{

class PostProcesser
{
public:
    PostProcesser();

    void resize(int width, int height);

    void postProcess(unsigned int renderedScene, unsigned int entityTexture, int selectedEntity);

    Engine::OpenGLFramebuffer &getFramebuffer();

private:
    // Trying out kernels based on https://learnopengl.com/Advanced-OpenGL/Framebuffers

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
                                 "layout (binding = 1) uniform isampler2D entityTexture;\n"
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

                                 "  int indices[9];\n"
                                 "  for(int i = 0; i < 9; i++)\n"
                                 "  {\n"
                                 "    indices[i] = texture(entityTexture, texCoord.st + offsets[i]).r;\n"
                                 "  }\n"
                                 "  vec3 col = vec3(0.0);\n"
                                 "  for(int i = 0; i < 9; i++) {\n"
                                 "    if (indices[i] == selectedEntity) {\n"
                                 "      col += vec3(1.0, 1.0, 0.0) * kernel[i];\n"
                                 "    } else {\n"
                                 "      col += vec3(0.0, 0.0, 0.0);\n"
                                 "    }"
                                 "  }"

                                 "  if (col.x > 0.001 || col.y > 0.001) {"
                                 "    FragColor = vec4(col, 1.0);"
                                 "  } else {"
                                 "    FragColor = texture(sceneTexture, texCoord);"
                                 "  }"
                                 "}";

    Engine::OpenGLFramebuffer m_frameBuffer{};
    unsigned int m_VBO{0};
    unsigned int m_EBO{0};
    unsigned int m_VAO{0};
    int m_indexPosition{-1};
    Engine::OpenGLProgram m_program{{{GL_VERTEX_SHADER, vertexShader}, {GL_FRAGMENT_SHADER, fragmentShader}}};
};

}; // namespace ModelerUtil

#endif