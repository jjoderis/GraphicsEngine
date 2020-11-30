#ifndef APPS_MODELER_IMGUI_IMGUI
#define APPS_MODELER_IMGUI_IMGUI

#include <cstring>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <Core/Math/math.h>
#include <Core/ECS/util.h>
#include <Util/fileHandling.h>
#include "Nodes/Camera/camera.h"
#include "Nodes/Geometry/geometry.h"
#include "Nodes/Material/material.h"
#include "Nodes/Render/render.h"
#include "Nodes/Transform/transform.h"
#include "Nodes/Entity/entity.h"
#include "Nodes/Light/light.h"
#include "Util/errorModal.h"

#include "../glfw/window.h"

class UI{
public:
    static void init(Engine::Registry& registry);
    static void preRender();
    static void render(Engine::Registry &registry);
    static void postRender();
};

#endif