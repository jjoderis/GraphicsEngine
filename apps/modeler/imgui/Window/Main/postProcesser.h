#ifndef APPS_MODELER_IMGUI_WINDOW_MAIN_POSTPROCESSER
#define APPS_MODELER_IMGUI_WINDOW_MAIN_POSTPROCESSER

#include <OpenGL/Renderer/renderer.h>
#include <OpenGL/Shader/shader.h>

namespace ModelerUtil {

class PostProcesser {
public:

  PostProcesser();

  void resize(int width, int height);

  void postProcess(unsigned int texture);

  Engine::OpenGLFramebuffer &getFramebuffer();

private:

  // Trying out kernels based on https://learnopengl.com/Advanced-OpenGL/Framebuffers

  const char* vertexShader = 
  "#version 330 core\n"
  "layout (location = 0) in vec3 pos;\n"
  "layout (location = 1) in vec2 tex;\n"
  "out vec2 texCoord;"
  "void main () {\n"
  "  texCoord = tex;"
  "  gl_Position = vec4(pos, 1);\n"
  "}\n";

  const char* fragmentShader =
  "#version 330 core\n"
  "out vec4 FragColor;\n"
  "in vec2 texCoord;\n"
  "uniform sampler2D texture1;\n"
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
      
  "  vec3 sampleTex[9];\n"
  "  for(int i = 0; i < 9; i++)\n"
  "  {\n"
  "    sampleTex[i] = vec3(texture(texture1, texCoord.st + offsets[i]));\n"
  "  }\n"
  "  vec3 col = vec3(0.0);\n"
  "  for(int i = 0; i < 9; i++)\n"
  "    col += sampleTex[i] * kernel[i];\n"
      
  "  FragColor = vec4(col, 1.0);\n"
  "}";  

  Engine::OpenGLFramebuffer m_frameBuffer{};
  unsigned int m_VBO{0};
  unsigned int m_EBO{0};
  unsigned int m_VAO{0};
  Engine::OpenGLProgram m_program{{ {GL_VERTEX_SHADER, vertexShader}, {GL_FRAGMENT_SHADER, fragmentShader} }};

};

};

#endif