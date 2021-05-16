#ifndef APPS_MODELER_IMGUI_UTIL_FILESYSTEM
#define APPS_MODELER_IMGUI_UTIL_FILESYSTEM

#include <filesystem>
#include <functional>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <vector>

namespace fs = std::filesystem;

namespace UIUtil
{
extern std::function<bool(const fs::path &)> can_open_function;
extern std::function<void(const fs::path &, const std::string &)> open_function;

void drawFileBrowser();
} // namespace UIUtil

#endif