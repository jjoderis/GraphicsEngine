#ifndef APPS_MODELER_IMGUI_NODES_LIGHT
#define APPS_MODELER_IMGUI_NODES_LIGHT

#include "../helpers.h"
#include "../Templates/componentWindow.h"

namespace Engine
{
class Registry;
} // namespace Engine
namespace UICreation
{

class LightComponentWindow : public ComponentWindow {

public:

    LightComponentWindow() = delete;
    LightComponentWindow(int &currentEntity, Engine::Registry &registry);

    virtual void render() override;

private: 

    virtual void main() override;

};

} // namespace UICreation

#endif