#include "geometry.h"

void UICreation::drawGeometryNode(Engine::Registry &registry) {
    Engine::GeometryComponent* geometry = registry.getComponent<Engine::GeometryComponent>(selectedEntity);

    createComponentNodeOutline<Engine::GeometryComponent>("Geometry", registry, geometry, [&]() {
        if (ImGui::TreeNode("Vertices")) { 
            std::vector<Engine::Math::Vector3> &vertices{geometry->getVertices()};
            for(int i = 0; i < vertices.size(); ++i) {
                std::string str = std::to_string(i);
                str.insert(0, "Vertex ");
                ImGui::DragFloat3(str.c_str(), vertices[i].raw(), 0.1);
                if (ImGui::IsItemEdited()) {
                    registry.updated<Engine::GeometryComponent>(selectedEntity);
                }
            }
            static Engine::Math::Vector3 newVertex{0.0, 0.0, 0.0};
            if (ImGui::Button("+##open_add_vertex_popup")) {
                ImGui::OpenPopup("Add Vertex");
            }
            if (ImGui::BeginPopup("Add Vertex"))
            {
                ImGui::InputFloat3("##new_vertex_input", newVertex.raw());
                ImGui::SameLine();
                if (ImGui::Button("+##add_vertex")) {
                    geometry->addVertex(Engine::Math::Vector3{newVertex});
                    registry.updated<Engine::GeometryComponent>(selectedEntity);
                    newVertex = Engine::Math::Vector3{ 0.0f, 0.0f, 0.0f };
                }
                ImGui::EndPopup();
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Faces")) {
            std::vector<unsigned int>& faces{ geometry->getFaces() };
            for(int i = 0; i < faces.size(); i += 3) {
                std::string str = std::to_string(i / 3);
                str.insert(0, "Face ");
                ImGui::InputScalarN(str.c_str(), ImGuiDataType_U32, faces.data() + i, 3);
                if (ImGui::IsItemEdited()) {
                    registry.updated<Engine::GeometryComponent>(selectedEntity);
                }
            }
            static unsigned int newFace[3]{0u, 0u, 0u};
            if (ImGui::Button("+##open_add_face_popup")) {
                ImGui::OpenPopup("Add Face");
            }
            if (ImGui::BeginPopup("Add Face"))
            {
                ImGui::InputScalarN("##new_face_input", ImGuiDataType_U32, newFace, 3);
                ImGui::SameLine();
                if (ImGui::Button("+##add_face")) {
                    geometry->addFace(newFace[0], newFace[1], newFace[2]);
                    registry.updated<Engine::GeometryComponent>(selectedEntity);
                    newFace[0] = 0u;
                    newFace[1] = 0u;
                    newFace[2] = 0u;
                }
                ImGui::EndPopup();
            }

            ImGui::TreePop();
        }
    });
}