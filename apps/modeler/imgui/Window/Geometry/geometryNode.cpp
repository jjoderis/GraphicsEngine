#include "geometryNode.h"
#include "../helpers.h"
#include <Components/Render/render.h>
#include <Core/Components/Geometry/geometry.h>
#include <OpenGL/Components/OpenGLGeometry/openGLGeometry.h>
#include <imgui.h>

extern bool dragging;

UICreation::GeometryComponentWindow::GeometryComponentWindow(int &currentEntity, Engine::Registry &registry)
    : TemplatedComponentWindow<Engine::GeometryComponent>{"Geometry", currentEntity, registry}
{
}

void UICreation::GeometryComponentWindow::main()
{
    ImGui::Button("Start Drag");
    createImGuiComponentDragSource<Engine::GeometryComponent>(m_component);

    if (!m_registry.hasComponent<Engine::RenderComponent>(m_selectedEntity))
    {
        ImGui::SameLine();
        if (ImGui::Button("Remove"))
        {
            m_registry.removeComponent<Engine::GeometryComponent>(m_selectedEntity);
        }
    }

    if (ImGui::TreeNode("Vertices"))
    {
        std::vector<Engine::Point3> &vertices{m_component->getVertices()};
        for (int i = 0; i < vertices.size(); ++i)
        {
            std::string str = std::to_string(i);
            str.insert(0, "Vertex ");
            ImGui::DragFloat3(str.c_str(), vertices[i].data(), 0.1);
            if (ImGui::IsItemEdited())
            {
                m_registry.updated<Engine::GeometryComponent>(m_selectedEntity);
            }
        }
        static Engine::Point3 newVertex{0.0, 0.0, 0.0};
        if (ImGui::Button("+##open_add_vertex_popup"))
        {
            ImGui::OpenPopup("Add Vertex");
        }
        if (ImGui::BeginPopup("Add Vertex"))
        {
            ImGui::InputFloat3("##new_vertex_input", newVertex.data());
            ImGui::SameLine();
            if (ImGui::Button("+##add_vertex"))
            {
                m_component->addVertex(Engine::Point3{newVertex});
                m_registry.updated<Engine::GeometryComponent>(m_selectedEntity);
                newVertex = Engine::Point3{0.0f, 0.0f, 0.0f};
            }
            ImGui::EndPopup();
        }

        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Faces"))
    {
        std::vector<unsigned int> &faces{m_component->getFaces()};
        for (int i = 0; i < faces.size(); i += 3)
        {
            std::string str = std::to_string(i / 3);
            str.insert(0, "Face ");
            ImGui::InputScalarN(str.c_str(), ImGuiDataType_U32, faces.data() + i, 3);
            if (ImGui::IsItemEdited())
            {
                m_registry.updated<Engine::GeometryComponent>(m_selectedEntity);
            }
        }
        static unsigned int newFace[3]{0u, 0u, 0u};
        if (ImGui::Button("+##open_add_face_popup"))
        {
            ImGui::OpenPopup("Add Face");
        }
        if (ImGui::BeginPopup("Add Face"))
        {
            ImGui::InputScalarN("##new_face_input", ImGuiDataType_U32, newFace, 3);
            ImGui::SameLine();
            if (ImGui::Button("+##add_face"))
            {
                m_component->addFace(newFace[0], newFace[1], newFace[2]);
                m_registry.updated<Engine::GeometryComponent>(m_selectedEntity);
                newFace[0] = 0u;
                newFace[1] = 0u;
                newFace[2] = 0u;
            }
            ImGui::EndPopup();
        }

        ImGui::TreePop();
    }
    if (ImGui::Button("Calculate Normals"))
    {
        m_component->calculateNormals();
        m_registry.updated<Engine::GeometryComponent>(m_selectedEntity);
    }
    if (ImGui::Button("Invert Normals"))
    {
        for (Engine::Vector3 &normal : m_component->getNormals())
        {
            normal = -normal;
        }
        m_registry.updated<Engine::GeometryComponent>(m_selectedEntity);
    }

    if (ImGui::Button("Center on origin"))
    {
        Engine::Vector3 avg{0.0, 0.0, 0.0};
        for (const auto &vert : m_component->getVertices())
        {
            avg += vert - Engine::Point3{0, 0, 0};
        }
        avg /= m_component->getVertices().size();

        for (auto &vert : m_component->getVertices())
        {
            vert -= avg;
        }
        m_registry.updated<Engine::GeometryComponent>(m_selectedEntity);
    }

    if (auto openGLGeometry = m_registry.getComponent<Engine::OpenGLGeometryComponent>(m_selectedEntity))
    {
        bool drawingPoints = openGLGeometry->drawingPoints();
        ImGui::Checkbox("Draw as points: ", &drawingPoints);
        if (ImGui::IsItemEdited())
        {
            openGLGeometry->drawPoints(drawingPoints);
        }
    }

    if (ImGui::Button("Calculate Bounding Box"))
    {
        m_component->calculateBoundingBox();
    }
}