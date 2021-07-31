#ifndef APPS_MODELER_IMGUI_WINDOW_CLASS
#define APPS_MODELER_IMGUI_WINDOW_CLASS

#include <string>

namespace UICreation {

class ImGuiWindow {

public:
  ImGuiWindow() = delete;

  // allows call to render the window
  virtual void render();

protected:
  std::string m_name;
  int m_width{0};
  int m_height{0};

  // prevent instances of this class
  ImGuiWindow(const std::string &name);

  // will begin the current window
  bool start();
  // renders the main window content (should be overwritten by child classes)
  virtual void main();
  // will render the end of the current window
  void end();

  virtual void checkUpdates();
  void onResize();

};

}

#endif