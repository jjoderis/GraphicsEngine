#ifndef APPS_MODELER_IMGUI_NODES_OPENGLSHADER
#define APPS_MODELER_IMGUI_NODES_OPENGLSHADER

#include "../../Util/errorModal.h"
#include "../../Util/fileBrowser.h"
#include "../helpers.h"
#include <Core/Components/Render/render.h>
#include <algorithm>
#include <misc/cpp/imgui_stdlib.h>

namespace UICreation
{

void drawShaderNode(Engine::Registry &registry);

}

#endif