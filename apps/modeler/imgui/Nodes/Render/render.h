#ifndef APPS_MODELER_IMGUI_NODES_RENDER
#define APPS_MODELER_IMGUI_NODES_RENDER

#include "../helpers.h"
#include <misc/cpp/imgui_stdlib.h>
#include <algorithm>
#include "../../Util/fileBrowser.h"
#include "../../Util/errorModal.h"

namespace UICreation {

void drawRenderNode(Engine::Registry& registry);

}

#endif