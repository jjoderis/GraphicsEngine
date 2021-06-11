#ifndef APPS_MODELER_GLAD_OPENGL
#define APPS_MODELER_GLAD_OPENGL

class OpenGL
{
public:
    static void init();

    static void preRender();
    static void render();
    static void postRender();
};

#endif