#ifndef APPS_MODELER_IMGUI_UTIL_FILESYSTEM
#define APPS_MODELER_IMGUI_UTIL_FILESYSTEM

#include <filesystem>
#include <functional>

namespace fs = std::filesystem;

namespace UIUtil
{
void initFileBrowserIcons();

extern std::function<bool(const fs::path &)> can_open_function;
extern std::function<void(const fs::path &, const std::string &)> open_function;

void openFileBrowser();
void drawFileBrowser();
} // namespace UIUtil

#endif