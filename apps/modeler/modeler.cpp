#include "glad/opengl.h"
#include "glfw/window.h"
#include "imgui/imgui.h"

#include <Math/math.h>
#include <ECS/registry.h>
#include <Core/Components/Material/material.h>
#include <Core/Components/Geometry/geometry.h>

#include <iostream>

GLFWwindow* Window::m_window = nullptr;

int main()
{
    Window::init();
    OpenGL::init();
    UI::init();

    // Engine::Math::Vector3 vertices[]{
    //     {-0.5f, -0.5f, 0.0f},
    //     {0.5f, -0.5f, 0.0f},
    //     {0.0f,  0.5f, 0.0f}
    // };

    // unsigned int VAO;
    // glGenVertexArrays(1, &VAO);
    // glBindVertexArray(VAO);

    // unsigned int VBO;
    // glGenBuffers(1, &VBO);

    // glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // glEnableVertexAttribArray(0);

    // const char *vertexShaderSource = "#version 330 core\n"
    // "layout (location = 0) in vec3 aPos;\n"
    // "uniform mat4 translation;\n"
    // "uniform vec4 color;\n"
    // "out vec4 fColor;\n"
    // "void main()\n"
    // "{\n"
    // "   fColor = color;"
    // "   gl_Position = translation * vec4(aPos, 1.0);\n"
    // "}\0";

    // unsigned int vertexShader;
    // vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    // glCompileShader(vertexShader);

    // int success;
    // char infoLog[512];
    // glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    // if (!success)
    // {
    //     glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    //     std::cerr << "ERROR::SHADER::VERTEX::COMPILATION:FAILED\n" << infoLog << '\n';
    //     exit(EXIT_FAILURE);
    // }

    // const char* fragmentShaderSource =
    // "#version 330 core\n"
    // "out vec4 FragColor;\n"
    // "in vec4 fColor;\n"
    // "void main()\n"
    // "{\n"
    // "    FragColor = fColor;\n"
    // "}\n";

    // unsigned int fragmentShader;
    // fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    // glCompileShader(fragmentShader);

    // glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    // if (!success)
    // {
    //     glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    //     std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION:FAILED\n" << infoLog << '\n';
    //     exit(EXIT_FAILURE);
    // }

    // unsigned int shaderProgram;
    // shaderProgram = glCreateProgram();
    // glAttachShader(shaderProgram, vertexShader);
    // glAttachShader(shaderProgram, fragmentShader);
    // glLinkProgram(shaderProgram);

    // glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    // if (!success) {
    //     glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    //     std::cerr << "ERROR::SHADER::PROGRAM::LINKING::FAILED\n" << infoLog << '\n';
    //     exit(EXIT_FAILURE);
    // }

    // glUseProgram(shaderProgram);

    // int transLoc = glGetUniformLocation(shaderProgram, "translation");
    // int colorLoc = glGetUniformLocation(shaderProgram, "color");

    // glDeleteShader(vertexShader);
    // glDeleteShader(fragmentShader);

    Engine::Math::Vector3 translation{ 0.0, 0.0, 0.0 };
    Engine::Math::Vector4 color{ 1.0f, 0.5f, 0.2f, 1.0f };

    Core::Registry registry{};

    unsigned int entityA = registry.addEntity();
    unsigned int entityB = registry.addEntity();

    registry.addComponent<Engine::TagComponent>(entityA, new Engine::TagComponent{"Mario"});
    registry.addComponent<Engine::MaterialComponent>(entityA, new Engine::MaterialComponent{ 0.5f, 0.2f, 0.1f, 1.0f });
    registry.addComponent<Engine::GeometryComponent>(entityA, new Engine::GeometryComponent{ 
        Engine::Math::Vector3{ -0.5, -0.5, 0.0 },
        Engine::Math::Vector3{ 0.5, -0.5, 0.0 },
        Engine::Math::Vector3{ 0.0, 0.5, 0.0 } 
    });

    registry.addComponent<Engine::TagComponent>(entityB, new Engine::TagComponent{"Luigi"});

    GLFWwindow* window = Window::getWindow();

    while (!glfwWindowShouldClose(window))
    {
        Window::preRender();

        OpenGL::preRender();

        UI::preRender();

        UI::render(registry);

        // glUseProgram(shaderProgram);

        // glUniformMatrix4fv(transLoc, 1, GL_FALSE, Engine::Math::getTranslation(translation).raw());
        // glUniform4fv(colorLoc, 1, color.raw());

        // glBindVertexArray(VAO);

        // glBufferSubData(GL_ARRAY_BUFFER, 0                , 3 * sizeof(float), vertices[0].raw());
        // glBufferSubData(GL_ARRAY_BUFFER, 3 * sizeof(float), 3 * sizeof(float), vertices[1].raw());
        // glBufferSubData(GL_ARRAY_BUFFER, 6 * sizeof(float), 3 * sizeof(float), vertices[2].raw());

        // glDrawArrays(GL_TRIANGLES, 0, 3);

        UI::postRender();

        Window::postRender();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}