#include "opengl.h"

void frameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void OpenGL::init() {
    if(!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize OpenGL context\n";
        exit(EXIT_FAILURE);
    }

    std::cout << "OpenGL Version: " << GLVersion.major << '.' << GLVersion.minor << '\n';

    GLFWwindow* window = Window::getWindow();

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
}

void OpenGL::preRender() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
void OpenGL::render() {}
void OpenGL::postRender() {} 