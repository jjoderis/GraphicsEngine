#ifndef APPS_MODELER_IMGUI_WINDOW_CLASS
#define APPS_MODELER_IMGUI_WINDOW_CLASS

#include <Core/Math/math.h>
#include <string>

namespace UICreation
{

class ImGuiWindow
{

public:
    ImGuiWindow() = delete;

    // allows call to render the window
    virtual void render();

protected:
    std::string m_name;
    int m_flags{0};
    bool m_open{NULL};

    // position of the window
    Engine::IVector2 m_pos{0, 0};
    // size of the windows content
    Engine::IVector2 m_size{0, 0};

    // prevent instances of this class
    ImGuiWindow(const std::string &name);

    // will begin the current window
    virtual bool start();
    // renders the main window content (should be overwritten by child classes)
    virtual void main();
    // will render the end of the current window
    virtual void end();

    virtual void checkUpdates();
    virtual void onResize();
};

} // namespace UICreation

#endif