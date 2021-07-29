#ifndef APPS_MODELER_IMGUI_IMGUI
#define APPS_MODELER_IMGUI_IMGUI

namespace Engine
{
namespace Util {
    class OpenGLTextureIndex;
}

class Registry;
}

class UI
{
public:
    static void init(Engine::Registry &registry, Engine::Util::OpenGLTextureIndex &textureIndex);
    static void preRender();
    static void render(Engine::Registry &registry);
    static void postRender();
};

#endif