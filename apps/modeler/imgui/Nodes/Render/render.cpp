#include "render.h"

std::vector<const char*> shaderTypes;
std::vector<Engine::OpenGLShader> shaders;
int current_selected_shader = 0;

void drawShaderEditModal(const std::shared_ptr<Engine::OpenGLRenderComponent>& render) {
    ImGui::SetNextWindowSize(ImVec2(520, 600));

    if (ImGui::BeginPopupModal("Shader Editor", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Combo("##shader_type", &current_selected_shader, shaderTypes.data(), shaderTypes.size());
        ImGui::InputTextMultiline("##shader_source_input", &shaders[current_selected_shader].m_source, ImVec2(500, 599));

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Use", ImVec2(120, 0))) {
            try {
                render->updateShaders(shaders);
            } catch (Engine::ShaderException& err) {
                errorMessage = err.what();
                ImGui::OpenPopup("Error Info");
            }
        }

        if (errorMessage.size()) {
            UICreation::drawErrorModal(errorMessage);
        }

        ImGui::EndPopup();
    }
}

void UICreation::drawRenderNode(Engine::Registry& registry) {
    if (std::shared_ptr<Engine::OpenGLRenderComponent> render = selectedRender.lock()) {
        createComponentNodeOutline<Engine::OpenGLRenderComponent>("Render", registry, render.get(), [&]() {
            if (ImGui::Button("Edit Shaders")) {
                shaders = render->getShaders();
                shaderTypes.clear();
                current_selected_shader = 0;

                for (Engine::OpenGLShader& shader: shaders) {
                    switch(shader.m_type) {
                        case GL_VERTEX_SHADER:
                            shaderTypes.push_back("Vertex Shader");
                            break;
                        case GL_FRAGMENT_SHADER:
                            shaderTypes.push_back("Fragment Shader");
                            break;
                        case GL_GEOMETRY_SHADER:
                            shaderTypes.push_back("Geometry Shader");
                            break;
                        case GL_TESS_EVALUATION_SHADER:
                            shaderTypes.push_back("Tesselation Evaluation Shader");
                            break;
                        case GL_TESS_CONTROL_SHADER:
                            shaderTypes.push_back("Tesselation Control Shader");
                            break;
                        case GL_COMPUTE_SHADER:
                            shaderTypes.push_back("Compute Shader");
                            break;
                    }
                } 
                ImGui::OpenPopup("Shader Editor");   
            }
            drawShaderEditModal(render);
            const char* types[2]{"Points\0", "Triangles\0"};
            static int primitive_type_current = 1;
            const char* comboLabel = types[primitive_type_current];
            static int primitive_type = GL_TRIANGLES;
            if (ImGui::RadioButton("Points", primitive_type == GL_POINTS)) {
                primitive_type = GL_POINTS;
                render->updatePrimitiveType(primitive_type);
            } 
            ImGui::SameLine();
            if (ImGui::RadioButton("Triangles", primitive_type == GL_TRIANGLES)) {
                primitive_type = GL_TRIANGLES;
                render->updatePrimitiveType(primitive_type);
            }
        });
    }
}