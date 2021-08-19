#ifndef APPS_MODELER_IMGUI_NODES_ENTITY
#define APPS_MODELER_IMGUI_NODES_ENTITY

#include <string>

namespace Engine
{
class Registry;
}
namespace UICreation
{
class EntityWindow
{
public:
    EntityWindow() = delete;
    EntityWindow(Engine::Registry &registry);

    void render();

private:
    Engine::Registry &m_registry;

    int m_changeNameEntity{-1};
    std::string m_currentName{};

    void drawEntityNode(unsigned int entity);
};
} // namespace UICreation

#endif