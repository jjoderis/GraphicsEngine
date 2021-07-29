#include "openGLShader.h"

#include "../../Util/errorModal.h"
#include "../../Util/fileBrowser.h"
#include <Core/Components/Render/render.h>
#include <OpenGL/Components/Shader/shader.h>
#include <algorithm>
#include <misc/cpp/imgui_stdlib.h>

extern bool dragging;

UICreation::OpenGLShaderComponentWindow::OpenGLShaderComponentWindow(int &currentEntity, Engine::Registry &registry)
    : TemplatedComponentWindow<Engine::OpenGLShaderComponent>{"Shader", currentEntity, registry}
{
}

void UICreation::OpenGLShaderComponentWindow::onComponentChange(std::shared_ptr<Engine::OpenGLShaderComponent> oldComponent) {
    TemplatedComponentWindow<Engine::OpenGLShaderComponent>::onComponentChange(oldComponent);

    loadShaders(m_component->getShaders());
}

void UICreation::OpenGLShaderComponentWindow::loadShader(const Engine::OpenGLShader &shader)
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

void UICreation::OpenGLShaderComponentWindow::loadShaders(const std::vector<Engine::OpenGLShader> &shaders)
{
    m_shaders.clear();
    m_shaderTypes.clear();
    m_currentShader = 0;

    for (const Engine::OpenGLShader &shader : shaders)
    {
        loadShader(shader);
    }
}

void UICreation::OpenGLShaderComponentWindow::drawShaderEditModal(unsigned int entity,
                         Engine::Registry &registry,
                         const std::shared_ptr<Engine::OpenGLShaderComponent> &shader)
{
    if (!m_modalOpen)
    {
        return;
    }

    if (ImGui::Begin("Shader Editor", &m_modalOpen))
    {
        ImGui::Combo("##shader_type", &m_currentShader, m_shaderTypes.data(), m_shaderTypes.size());

        ImGui::SameLine();
        if (ImGui::Button("Import##single_shader"))
        {
            UIUtil::can_open_function = [this](const fs::path &path) -> bool
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

                return (fs::is_regular_file(path) && m_shaders[m_currentShader].m_type == fileType);
            };
            UIUtil::open_function = [this](const fs::path &path, const std::string &fileName)
            { loadShader(Engine::loadShader(path)); };
            UIUtil::openFileBrowser();
        }

        ImVec2 pos{ImGui::GetWindowContentRegionMin()};
        ImVec2 end{ImGui::GetWindowContentRegionMax()};
        ImVec2 textSize{pos.x + end.x, pos.y + end.y - 100};

        ImGui::InputTextMultiline("##shader_source_input", &m_shaders[m_currentShader].m_source, textSize);

        if (ImGui::Button("Close", ImVec2(120, 0)))
        {
            m_modalOpen = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Import##multi_shader", ImVec2(120, 0)))
        {
            UIUtil::can_open_function = [](const fs::path &path) -> bool { return fs::is_directory(path); };
            UIUtil::open_function = [this](const fs::path &path, const std::string &fileName)
            { loadShaders(Engine::loadShaders(path.c_str())); };
            UIUtil::openFileBrowser();
        }
        ImGui::SameLine();
        if (ImGui::Button("Export##mulit_shader", ImVec2(120, 0)))
        {
            UIUtil::can_open_function = [](const fs::path &path) -> bool { return fs::is_directory(path); };
            UIUtil::open_function = [this](const fs::path &path, const std::string &fileName)
            { Engine::saveShaders(path, m_shaders); };
            UIUtil::openFileBrowser();
        }
        ImGui::SameLine();
        if (ImGui::Button("Use", ImVec2(120, 0)))
        {
            try
            {
                shader->updateShaders(m_shaders);
                registry.updated<Engine::OpenGLShaderComponent>(entity);
            }
            catch (Engine::ShaderException &err)
            {
                errorMessage = err.what();
                ImGui::OpenPopup("Error Info");
            }
        }

        UIUtil::drawErrorModal(errorMessage);

    }
    ImGui::End();
}

void UICreation::OpenGLShaderComponentWindow::main() {
    createImGuiComponentDragSource<Engine::OpenGLShaderComponent>(dragging);

    if (ImGui::Button("Edit Shaders"))
    {
        loadShaders(m_component->getShaders());
        m_modalOpen = 1;
    }
    drawShaderEditModal(selectedEntity, m_registry, m_component);
    const char *types[2]{"Points\0", "Triangles\0"};
    static int primitive_type_current = 1;
    const char *comboLabel = types[primitive_type_current];
    static int primitive_type = GL_TRIANGLES;

    bool isRendered = m_registry.hasComponent<Engine::RenderComponent>(selectedEntity);
    ImGui::Checkbox("Render", &isRendered);
    if (ImGui::IsItemEdited())
    {
        if (!isRendered)
        {
            m_registry.removeComponent<Engine::RenderComponent>(selectedEntity);
        }
        else
        {
            m_registry.createComponent<Engine::RenderComponent>(selectedEntity);
        }
    }
}
