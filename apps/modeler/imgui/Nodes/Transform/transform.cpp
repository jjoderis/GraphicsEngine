#include "transform.h"

void UICreation::drawTransformNode(Engine::Registry &registry) {
    Engine::TransformComponent* transform = registry.getComponent<Engine::TransformComponent>(selectedEntity);

    createComponentNodeOutline<Engine::TransformComponent>("Transform", registry, transform, [&]() {
        ImGui::DragFloat3("Translation", transform->getTranslation().raw(), 0.1);
        if(ImGui::IsItemEdited()) {
            transform->update();
            registry.updated<Engine::TransformComponent>(selectedEntity);
        }
        ImGui::DragFloat3("Scaling", transform->getScaling().raw(), 0.1);
        if(ImGui::IsItemEdited()) {
            transform->update();
            registry.updated<Engine::TransformComponent>(selectedEntity);
        }
        
        auto rotDeg = MathLib::Util::radToDeg(transform->getRotation());
        ImGui::DragFloat3("Rotation", rotDeg.raw(), 1.0);
        if(ImGui::IsItemEdited()) {
            transform->setRotation(MathLib::Util::degToRad(rotDeg));
            transform->update();
            registry.updated<Engine::TransformComponent>(selectedEntity);
        }
    });
}