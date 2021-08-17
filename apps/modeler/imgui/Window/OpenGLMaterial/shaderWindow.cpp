#include "shaderWindow.h"

#include "../../Util/errorModal.h"
#include "../helpers.h"
#include <OpenGL/Components/Shader/shader.h>
#include <filesystem>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

namespace fs = std::filesystem;

UICreation::OpenGLShaderCodeWindow::OpenGLShaderCodeWindow(int &currentEntity, Engine::Registry &registry)
    : TemplatedComponentWindow<Engine::OpenGLShaderComponent>{"Shader Editor", currentEntity, registry}
{
    m_open = false;
}

void UICreation::OpenGLShaderCodeWindow::onComponentChange(std::shared_ptr<Engine::OpenGLShaderComponent> oldShader)
{
    m_shaderTypes.clear();
    m_shaders.clear();
    m_currentShader = 0;

    if (m_component)
    {
        loadShaders(m_component->getShaders());
    }
}

void UICreation::OpenGLShaderCodeWindow::loadShader(const Engine::OpenGLShader &shader)
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
        m_shaders.begin(), m_shaders.end(), [&shader](Engine::OpenGLShader &el) { return el.m_type == shader.m_type; });
    if (it == m_shaders.end())
    {
        m_shaders.push_back(shader);
        m_shaderTypes.push_back(shaderType);
    }
    else
    {
        // there is already a shader of this type != replace source
        it->m_source = shader.m_source;
    }
}

void UICreation::OpenGLShaderCodeWindow::loadShaders(const std::vector<Engine::OpenGLShader> &shaders)
{
    m_shaders.clear();
    m_shaderTypes.clear();
    m_currentShader = 0;

    for (const Engine::OpenGLShader &shader : shaders)
    {
        loadShader(shader);
    }
}

bool isShaderFile(const fs::path &path)
{
    try
    {
        Engine::shaderPathToType(path);
    }
    catch (Engine::ShaderException &err)
    {
        return false;
    }

    return true;
}

bool isShaderDirectory(const fs::path &path)
{
    if (!fs::is_directory(path))
    {
        return false;
    }

    for (const auto &entry : fs::directory_iterator(path.c_str()))
    {
        if (!isShaderFile(entry))
        {
            return false;
        }
    }

    return true;
}

void UICreation::OpenGLShaderCodeWindow::main()
{
    ImGui::Combo("##shader_type", &m_currentShader, m_shaderTypes.data(), m_shaderTypes.size());

    ImVec2 pos{ImGui::GetWindowContentRegionMin()};
    ImVec2 end{ImGui::GetWindowContentRegionMax()};
    ImVec2 textSize{pos.x + end.x, pos.y + end.y - 100};

    ImGui::InputTextMultiline("##shader_source_input", &m_shaders[m_currentShader].m_source, textSize);

    if (auto path = createImGuiHighlightedDropTarget<fs::path>(
            "system_path_payload",
            [](const fs::path &path) { return (isShaderFile(path) || isShaderDirectory(path)); }))
    {
        if (fs::is_directory(*path))
        {
            loadShaders(Engine::loadShaders(path->c_str()));
        }
        else
        {
            loadShader(Engine::loadShader(*path));
        }
    }

    if (ImGui::Button("Close", ImVec2(120, 0)))
    {
        m_open = false;
    }
    ImGui::SameLine();
    ImGui::Button("Export##multi_shader", ImVec2(120, 0));

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        ImGui::SetDragDropPayload("shaders_payload", &m_shaders, sizeof(std::vector<Engine::OpenGLShader>));

        ImGui::Text("Shaders");
        ImGui::EndDragDropSource();
    }

    ImGui::SameLine();
    if (ImGui::Button("Use", ImVec2(120, 0)))
    {
        try
        {
            m_component->updateShaders(m_shaders);
            m_registry.updated<Engine::OpenGLShaderComponent>(m_currentEntity);
        }
        catch (Engine::ShaderException &err)
        {
            errorMessage = err.what();
            ImGui::OpenPopup("Error Info");
        }
    }

    UIUtil::drawErrorModal(errorMessage);
}

void UICreation::OpenGLShaderCodeWindow::open() { m_open = true; }

std::shared_ptr<Engine::OpenGLShaderComponent> UICreation::OpenGLShaderCodeWindow::getShader() { return m_component; }

void UICreation::OpenGLShaderCodeWindow::render()
{
    if (!m_open)
    {
        return;
    }

    TemplatedComponentWindow<Engine::OpenGLShaderComponent>::render();
}