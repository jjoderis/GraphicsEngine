#ifndef APPS_MODELER_IMGUI_IMGUI
#define APPS_MODELER_IMGUI_IMGUI

#include "Nodes/Camera/camera.h"
#include "Nodes/Entity/entity.h"
#include "Nodes/Geometry/geometryNode.h"
#include "Nodes/Light/light.h"
#include "Nodes/Material/material.h"
#include "Nodes/OpenGLShader/openGLShader.h"
#include "Nodes/Transform/transform.h"
#include "Util/errorModal.h"
#include <Core/ECS/util.h>
#include <Core/Math/math.h>
#include <Util/fileHandling.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <cstring>
#include <imgui.h>

#include "../glfw/window.h"

class UI
{
public:
    static void init(Engine::Registry &registry);
    static void preRender();
    static void render(Engine::Registry &registry);
    static void postRender();
};

#endif