#ifndef APPS_MODELER_IMGUI_IMGUI
#define APPS_MODELER_IMGUI_IMGUI

namespace Engine
{
class Registry;
}

class UI
{
public:
    static void init(Engine::Registry &registry);
    static void preRender();
    static void render(Engine::Registry &registry);
    static void postRender();
};

#endif