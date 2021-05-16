#ifndef APPS_MODELER_GLAD_OPENGL
#define APPS_MODELER_GLAD_OPENGL

#include "../glfw/window.h"
#include <cstdlib>
#include <glad/glad.h>
#include <iostream>

class OpenGL
{
public:
    static void init();

    static void preRender();
    static void render();
    static void postRender();
};

#endif