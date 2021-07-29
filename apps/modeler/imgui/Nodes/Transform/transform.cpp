#include "transform.h"
#include <Core/Components/Transform/transform.h>

extern bool dragging;

UICreation::TransformComponentWindow::TransformComponentWindow(int &currentEntity, Engine::Registry &registry)
    : TemplatedComponentWindow<Engine::TransformComponent>{"Transform", currentEntity, registry}
{    
}

void UICreation::TransformComponentWindow::main() {
    createImGuiComponentDragSource<Engine::TransformComponent>(dragging);

    ImGui::DragFloat3("Translation", m_component->getTranslation().raw(), 0.1);
    if (ImGui::IsItemEdited())
    {
        m_component->update();
        m_registry.updated<Engine::TransformComponent>(selectedEntity);
    }
    ImGui::DragFloat3("Scaling", m_component->getScaling().raw(), 0.1);
    if (ImGui::IsItemEdited())
    {
        m_component->update();
        m_registry.updated<Engine::TransformComponent>(selectedEntity);
    }

    auto rotDeg = MathLib::Util::radToDeg(m_component->getRotation());
    ImGui::DragFloat3("Rotation", rotDeg.raw(), 1.0);
    if (ImGui::IsItemEdited())
    {
        m_component->setRotation(MathLib::Util::degToRad(rotDeg));
        m_component->update();
        m_registry.updated<Engine::TransformComponent>(selectedEntity);
    }
}