#ifndef APPS_MODELER_GLFW
#define APPS_MODELER_GLFW

class GLFWwindow;
class Window
{
public:
    Window() = delete;

    static void init();
    static GLFWwindow *getWindow();
    static void preRender();
    static void render();
    static void postRender();

private:
    static GLFWwindow *m_window;
};

#endif