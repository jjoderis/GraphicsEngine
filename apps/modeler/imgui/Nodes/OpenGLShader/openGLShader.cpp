#include "openGLShader.h"

#include "../../Util/errorModal.h"
#include "../../Util/fileBrowser.h"
#include <Core/Components/Render/render.h>
#include <OpenGL/Components/Shader/shader.h>
#include <algorithm>
#include <misc/cpp/imgui_stdlib.h>

std::vector<const char *> g_shaderTypes;
std::vector<Engine::OpenGLShader> g_shaders;
int current_selected_shader = 0;

extern bool dragging;

void loadShader(const Engine::OpenGLShader &shader)
{
    const char *shaderType{};

    switch (shader.m_type)
    {
    case GL_VERTEX_SHADER:
        shaderType = "Vertex Shader";
        break;
    case GL_FRAGMENT_SHADER:
        shaderType = "Fragment Shader";
        break;
    case GL_GEOMETRY_SHADER:
        shaderType = "Geometry Shader";
        break;
    case GL_TESS_EVALUATION_SHADER:
        shaderType = "Tesselation Evaluation Shader";
        break;
    case GL_TESS_CONTROL_SHADER:
        shaderType = "Tesselation Control Shader";
        break;
    case GL_COMPUTE_SHADER:
        shaderType = "Compute Shader";
        break;
    }

    // if there isn't a shader of this type already => just add
    auto it = std::find_if(
        g_shaders.begin(), g_shaders.end(), [&shader](Engine::OpenGLShader &el) { return el.m_type == shader.m_type; });
    if (it == g_shaders.end())
    {
        g_shaders.push_back(shader);
        g_shaderTypes.push_back(shaderType);
    }
    else
    {
        // there is already a shader of this type != replace source
        it->m_source = shader.m_source;
    }
}

void loadShaders(const std::vector<Engine::OpenGLShader> &shaders)
{
    g_shaders.clear();
    g_shaderTypes.clear();
    current_selected_shader = 0;

    for (const Engine::OpenGLShader &shader : shaders)
    {
        loadShader(shader);
    }
}

void drawShaderEditModal(unsigned int entity,
                         Engine::Registry &registry,
                         const std::shared_ptr<Engine::OpenGLShaderComponent> &shader)
{
    ImGui::SetNextWindowSize(ImVec2(520, 600));

    if (ImGui::BeginPopupModal("Shader Editor", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Combo("##shader_type", &current_selected_shader, g_shaderTypes.data(), g_shaderTypes.size());

        ImGui::SameLine();
        if (ImGui::Button("Import##single_shader"))
        {
            UIUtil::can_open_function = [](const fs::path &path) -> bool
            {
                // we can import the file if the path corresponds to a file that has the correct fileExtension for the
                // shader type
                GLenum fileType;

                try
                {
                    fileType = Engine::shaderPathToType(path);
                }
                catch (Engine::ShaderException &err)
                {
                    fileType = GL_NONE;
                }

                return (fs::is_regular_file(path) && g_shaders[current_selected_shader].m_type == fileType);
            };
            UIUtil::open_function = [](const fs::path &path, const std::string &fileName)
            { loadShader(Engine::loadShader(path)); };
            ImGui::OpenPopup("File Browser");
        }

        ImGui::InputTextMultiline(
            "##shader_source_input", &g_shaders[current_selected_shader].m_source, ImVec2(500, 599));

        if (ImGui::Button("Close", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Import##multi_shader", ImVec2(120, 0)))
        {
            UIUtil::can_open_function = [](const fs::path &path) -> bool { return fs::is_directory(path); };
            UIUtil::open_function = [](const fs::path &path, const std::string &fileName)
            { loadShaders(Engine::loadShaders(path.c_str())); };
            ImGui::OpenPopup("File Browser");
        }
        ImGui::SameLine();
        if (ImGui::Button("Export##mulit_shader", ImVec2(120, 0)))
        {
            UIUtil::can_open_function = [](const fs::path &path) -> bool { return fs::is_directory(path); };
            UIUtil::open_function = [](const fs::path &path, const std::string &fileName)
            { Engine::saveShaders(path, g_shaders); };
            ImGui::OpenPopup("File Browser");
        }
        ImGui::SameLine();
        if (ImGui::Button("Use", ImVec2(120, 0)))
        {
            try
            {
                shader->updateShaders(g_shaders);
                registry.updated<Engine::OpenGLShaderComponent>(entity);
            }
            catch (Engine::ShaderException &err)
            {
                errorMessage = err.what();
                ImGui::OpenPopup("Error Info");
            }
        }

        UIUtil::drawFileBrowser();

        UIUtil::drawErrorModal(errorMessage);

        ImGui::EndPopup();
    }
}

template <>
void UICreation::createComponentNodeMain<Engine::OpenGLShaderComponent>(
    std::shared_ptr<Engine::OpenGLShaderComponent> shader, Engine::Registry &registry)
{
    createImGuiComponentDragSource<Engine::OpenGLShaderComponent>(dragging);

    if (ImGui::Button("Edit Shaders"))
    {
        loadShaders(shader->getShaders());
        ImGui::OpenPopup("Shader Editor");
    }
    drawShaderEditModal(selectedEntity, registry, shader);
    const char *types[2]{"Points\0", "Triangles\0"};
    static int primitive_type_current = 1;
    const char *comboLabel = types[primitive_type_current];
    static int primitive_type = GL_TRIANGLES;

    bool isRendered = registry.hasComponent<Engine::RenderComponent>(selectedEntity);
    ImGui::Checkbox("Render", &isRendered);
    if (ImGui::IsItemEdited())
    {
        if (!isRendered)
        {
            registry.removeComponent<Engine::RenderComponent>(selectedEntity);
        }
        else
        {
            registry.addComponent<Engine::RenderComponent>(selectedEntity, std::make_shared<Engine::RenderComponent>());
        }
    }
    // if (ImGui::RadioButton("Points", primitive_type
    // == GL_POINTS))
    // {
    //     primitive_type = GL_POINTS;
    //     render->updatePrimitiveType(primitive_type);
    // }
    // ImGui::SameLine();
    // if (ImGui::RadioButton("Triangles",
    // primitive_type == GL_TRIANGLES))
    // {
    //     primitive_type = GL_TRIANGLES;
    //     render->updatePrimitiveType(primitive_type);
    // }
}