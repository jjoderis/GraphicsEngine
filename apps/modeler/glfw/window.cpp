#include "window.h"

#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>

void errorCallback(int error, const char *description)
{
    std::cerr << "GLFW encountered an error:\n" << description << '\n';
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void Window::init()
{
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    m_window = glfwCreateWindow(1280, 960, "Simple example", NULL, NULL);

    if (!m_window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(m_window, keyCallback);

    glfwMakeContextCurrent(m_window);

    glfwSwapInterval(1);
}

GLFWwindow *Window::getWindow() { return m_window; }

void Window::preRender() { glfwPollEvents(); }

void Window::render() {}

void Window::postRender() { glfwSwapBuffers(m_window); }