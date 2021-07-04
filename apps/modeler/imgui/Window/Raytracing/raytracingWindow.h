#ifndef APPS_MODELER_IMGUI_WINDOW_RAYTRACING
#define APPS_MODELER_IMGUI_WINDOW_RAYTRACING

namespace Engine
{
class Registry;
}

namespace UICreation
{

void showRaytracingWindow(Engine::Registry &registry);
void hideRaytracingWindow();

void drawRaytracingWindow();

} // namespace UICreation

#endif