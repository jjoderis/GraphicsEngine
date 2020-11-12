#ifndef APPS_MODELER_IMGUI_IMGUI
#define APPS_MODELER_IMGUI_IMGUI

#include <cstring>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <Core/Math/math.h>
#include <Core/ECS/registry.h>
#include <Core/ECS/util.h>
#include <Core/Components/Tag/tag.h>
#include <Core/Components/Material/material.h>
#include <Core/Components/Geometry/geometry.h>
#include <Core/Components/Transform/transform.h>
#include <OpenGL/Components/Render/render.h>

#include "../glfw/window.h"

class UI{
public:
    static void init();
    static void preRender();
    static void render(Engine::Registry &registry);
    static void postRender();
};

#endif