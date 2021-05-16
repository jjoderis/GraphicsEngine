#ifndef APPS_MODELER_GLAD_OPENGL
#define APPS_MODELER_GLAD_OPENGL

// clang-format off
#include <glad/glad.h>
#include "../glfw/window.h"
#include <cstdlib>
#include <iostream>
// clang-format on

class OpenGL
{
public:
    static void init();

    static void preRender();
    static void render();
    static void postRender();
};

#endif