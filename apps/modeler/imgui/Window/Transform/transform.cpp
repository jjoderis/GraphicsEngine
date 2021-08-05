#include "transform.h"
#include <Core/Components/Transform/transform.h>
#include <imgui.h>

extern bool dragging;

UICreation::TransformComponentWindow::TransformComponentWindow(int &currentEntity, Engine::Registry &registry)
    : TemplatedComponentWindow<Engine::TransformComponent>{"Transform", currentEntity, registry}
{
    m_transformChangeCallback = registry.onUpdate<Engine::TransformComponent>([this](unsigned int entity, std::weak_ptr<Engine::TransformComponent> transform) {
        if (this->m_component == transform.lock()) {
            this->updateInternals();
        }
    });
}

void UICreation::TransformComponentWindow::updateInternals() {
    m_quat = m_component->getRotation();
    m_euler = MathLib::Util::radToDeg(m_component->getEulerRotation());

    auto angle{ MathLib::Util::radToDeg(2 * acos(m_quat.qw()))  };
    m_axisAngle = Engine::Math::Vector4{m_quat.qv(), angle};
}

void UICreation::TransformComponentWindow::onComponentChange(std::shared_ptr<Engine::TransformComponent> oldComponent) {
    updateInternals();
}

const char* possible_rotation_systems[]{
    "Euler",
    "Angle Axis",
    "Quaternion"
};

int possible_rotation_system_current = 0;

void UICreation::TransformComponentWindow::main() {
    ImGui::DragFloat3("Translation", m_component->getTranslation().raw(), 0.1);
    if (ImGui::IsItemEdited())
    {
        m_component->update();
        m_registry.updated<Engine::TransformComponent>(m_selectedEntity);
    }
    ImGui::DragFloat3("Scaling", m_component->getScaling().raw(), 0.1);
    if (ImGui::IsItemEdited())
    {
        m_component->update();
        m_registry.updated<Engine::TransformComponent>(m_selectedEntity);
    }

    ImGui::Separator();

    ImGui::Text("Rotation");

    if (ImGui::BeginCombo("##Rotation System", possible_rotation_systems[possible_rotation_system_current])) {
        if (ImGui::Selectable(possible_rotation_systems[0], possible_rotation_system_current == 0)) {
            possible_rotation_system_current = 0;
        }
        if (ImGui::Selectable(possible_rotation_systems[1], possible_rotation_system_current == 1)) {
            possible_rotation_system_current = 1;
        }
        if (ImGui::Selectable(possible_rotation_systems[2], possible_rotation_system_current == 2)) {
            possible_rotation_system_current = 2;
        }

        ImGui::EndCombo();
    }

    if (possible_rotation_systems[possible_rotation_system_current] == "Euler") {
        drawEuler();
    }

    if (possible_rotation_systems[possible_rotation_system_current] == "Angle Axis") {
        drawAngleAxis();
    }

    if (possible_rotation_systems[possible_rotation_system_current] == "Quaternion") {
        drawQuaternion();
    }
}

void UICreation::TransformComponentWindow::drawEuler() {
    bool wasUpdated = false;
    wasUpdated |= ImGui::DragFloat("x", &m_euler.at(0), 1.0);
    wasUpdated |= ImGui::DragFloat("y", &m_euler.at(1), 1.0);
    wasUpdated |= ImGui::DragFloat("z", &m_euler.at(2), 1.0);

    if (wasUpdated) {
        auto rad = MathLib::Util::degToRad(m_euler);
        m_component->setRotation(rad);
        m_component->update();
        m_registry.updated<Engine::TransformComponent>(m_selectedEntity);
    }
}

void UICreation::TransformComponentWindow::drawAngleAxis() {
    bool wasUpdated = false;
    wasUpdated |= ImGui::DragFloat("Angle", &m_axisAngle.at(3), 1.0);
    wasUpdated |= ImGui::DragFloat("x", &m_axisAngle.at(0), 0.1);
    wasUpdated |= ImGui::DragFloat("y", &m_axisAngle.at(1), 0.1);
    wasUpdated |= ImGui::DragFloat("z", &m_axisAngle.at(2), 0.1);

    if (wasUpdated) {
        m_component->setRotation(Engine::Math::Quaternion{}.setRotation(m_axisAngle, MathLib::Util::degToRad(m_axisAngle.at(3))));
        m_component->update();
        m_registry.updated<Engine::TransformComponent>(m_selectedEntity);
    }
}

void UICreation::TransformComponentWindow::drawQuaternion() {
    bool wasUpdated = false;
    wasUpdated |= ImGui::DragFloat("w", &m_quat.qw(), 0.1);
    wasUpdated |= ImGui::DragFloat("x", &m_quat.qx(), 0.1);
    wasUpdated |= ImGui::DragFloat("y", &m_quat.qy(), 0.1);
    wasUpdated |= ImGui::DragFloat("z", &m_quat.qz(), 0.1);

    if (wasUpdated) {
        m_component->setRotation(m_quat);
        m_component->update();
        m_registry.updated<Engine::TransformComponent>(m_selectedEntity);
    }
}