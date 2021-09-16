// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../window/window.h"
#include <cstdlib>
#include <iostream>
// clang-format on

void GLAPIENTRY MessageCallback(GLenum source,
                                GLenum type,
                                GLuint id,
                                GLenum severity,
                                GLsizei length,
                                const GLchar *message,
                                const void *userParam)
{
    (void)type;
    (void)id;
    (void)length;
    (void)userParam;
    (void)source;
    if (type == GL_DEBUG_TYPE_ERROR)
    {
        fprintf(stderr,
                "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
                (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
                type,
                severity,
                message);
    }
}

void frameBufferSizeCallback(GLFWwindow *window, int width, int height) { glViewport(0, 0, width, height); }

void initOpenGL()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize OpenGL context\n";
        exit(EXIT_FAILURE);
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(MessageCallback, 0);

    std::cout << "OpenGL Version: " << GLVersion.major << '.' << GLVersion.minor << '\n';

    GLFWwindow *window = getWindow();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);

    glPointSize(2.0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
}