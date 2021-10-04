#include "textureHelper.h"

#include "../window/window.h"
#include "./shaderHelper.h"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <cstddef>

unsigned int program;
unsigned int VAO;

void initVisualizer()
{
    const char *vertexShader = "#version 430 core\n"
                               "layout (location = 0) in vec3 pos;\n"
                               "layout (location = 1) in vec2 tex;\n"
                               "out vec2 texCoord;"
                               "void main () {\n"
                               "  texCoord = tex;"
                               "  gl_Position = vec4(pos, 1);\n"
                               "}\n";

    const char *fragmentShader = "#version 430 core\n"
                                 "in vec2 texCoord;\n"
                                 "uniform sampler2D sceneTexture;\n"

                                 "out vec4 FragColor;"

                                 "void main()\n"
                                 "{\n"
                                 "  FragColor = texture(sceneTexture, texCoord);"
                                 "}";

    auto vert{glCreateShader(GL_VERTEX_SHADER)};
    glShaderSource(vert, 1, &vertexShader, NULL);
    glCompileShader(vert);

    checkShader(vert);

    auto frag{glCreateShader(GL_FRAGMENT_SHADER)};
    glShaderSource(frag, 1, &fragmentShader, NULL);
    glCompileShader(frag);

    checkShader(frag);

    program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    checkProgram(program);

    float vertices[]{-1, -1, 0, 0, 0, 1, -1, 0, 1, 0, 1, 1, 0, 1, 1, -1, 1, 0, 0, 1};

    int indices[]{0, 1, 2, 0, 2, 3};

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void visualizeTexture(unsigned int texture)
{
    auto window{getWindow()};

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(VAO);
        glUseProgram(program);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
    }
}

unsigned int createOutputTexture(int width, int height)
{
    unsigned int outTexture;
    glGenTextures(1, &outTexture);
    glBindTexture(GL_TEXTURE_2D, outTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);

    return outTexture;
}