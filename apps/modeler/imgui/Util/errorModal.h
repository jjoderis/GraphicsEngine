#ifndef APPS_MODELER_IMGUI_UTIL_ERRORMODAL
#define APPS_MODELER_IMGUI_UTIL_ERRORMODAL

#include <string>
#include <imgui.h>

extern std::string errorMessage;
extern bool errorModalRendered;

namespace UIUtil {
    void drawErrorModal(std::string& errorMessage);
}

#endif