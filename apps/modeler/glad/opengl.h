#ifndef APPS_MODELER_GLAD_OPENGL
#define APPS_MODELER_GLAD_OPENGL

#include <glad/glad.h>
#include "../glfw/window.h"
#include <iostream>
#include <cstdlib>

class OpenGL {
public:
    static void init();  

    static void preRender();
    static void render();
    static void postRender();  
};

#endif