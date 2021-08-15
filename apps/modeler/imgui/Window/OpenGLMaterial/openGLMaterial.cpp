#include "openGLMaterial.h"

#include "../../Util/errorModal.h"
#include "../helpers.h"
#include <Components/Render/render.h>
#include <Core/ECS/registry.h>
#include <OpenGL/Components/Material/material.h>
#include <OpenGL/Components/Texture/texture.h>
#include <OpenGL/Util/textureIndex.h>
#include <Raytracing/Components/Material/raytracingMaterial.h>
#include <filesystem>
#include <glad/glad.h>
#include <misc/cpp/imgui_stdlib.h>

namespace fs = std::filesystem;

extern bool dragging;

UICreation::MaterialComponentWindow::MaterialComponentWindow(int &currentEntity,
                                                             Engine::Registry &registry,
                                                             Engine::Util::OpenGLTextureIndex &textureIndex)
    : ComponentWindow{"Material", currentEntity, registry}, m_textureIndex{textureIndex}
{
}

void UICreation::MaterialComponentWindow::onEntityChange(int oldEntity)
{
    ComponentWindow::onEntityChange(oldEntity);

    checkShaderChange();
}

void UICreation::MaterialComponentWindow::checkUpdates()
{
    ComponentWindow::checkUpdates();

    checkShaderChange();
}

void UICreation::MaterialComponentWindow::checkShaderChange()
{
    auto currentShader{m_registry.getComponent<Engine::OpenGLShaderComponent>(m_currentEntity)};
    if (currentShader != m_shader)
    {
        m_shader = currentShader;
        if (m_shader)
        {
            loadShaders(m_shader->getShaders());
        }
    }
}

void UICreation::MaterialComponentWindow::render()
{
    bool hasOpenGLShader = m_registry.hasComponent<Engine::OpenGLShaderComponent>(m_selectedEntity);
    bool hasRaytraceMaterial = m_registry.hasComponent<Engine::RaytracingMaterial>(m_selectedEntity);
    if (hasOpenGLShader || hasRaytraceMaterial)
    {
        ComponentWindow::render();
    }
}

bool isImage(const fs::path &path)
{
    fs::path extension{path.extension().c_str()};

    return extension == ".jpg" || extension == ".jpeg" || extension == ".png";
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

void UICreation::MaterialComponentWindow::loadShader(const Engine::OpenGLShader &shader)
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

void UICreation::MaterialComponentWindow::loadShaders(const std::vector<Engine::OpenGLShader> &shaders)
{
    m_shaders.clear();
    m_shaderTypes.clear();
    m_currentShader = 0;

    for (const Engine::OpenGLShader &shader : shaders)
    {
        loadShader(shader);
    }
}

void UICreation::MaterialComponentWindow::drawShaderEditModal(
    unsigned int entity, Engine::Registry &registry, const std::shared_ptr<Engine::OpenGLShaderComponent> &shader)
{
    if (!m_modalOpen)
    {
        return;
    }

    if (ImGui::Begin("Shader Editor", &m_modalOpen))
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
            m_modalOpen = 0;
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

void UICreation::MaterialComponentWindow::main()
{

    if (auto shader{m_registry.getComponent<Engine::OpenGLShaderComponent>(m_currentEntity)})
    {
        ImGui::Text("OpenGL Shader");
        ImGui::SameLine();
        ImGui::Button("Drag##OpenGLShader");
        createImGuiComponentDragSource<Engine::OpenGLShaderComponent>(m_shader);
        if (!m_registry.hasComponent<Engine::RenderComponent>(m_currentEntity))
        {
            ImGui::SameLine();
            if (ImGui::Button("x##OpenGLShader"))
            {
                m_registry.removeComponent<Engine::OpenGLShaderComponent>(m_currentEntity);
            }
        }

        if (ImGui::Button("Edit Shaders"))
        {
            loadShaders(m_shader->getShaders());
            m_modalOpen = 1;
        }
        drawShaderEditModal(m_selectedEntity, m_registry, m_shader);
        const char *types[2]{"Points\0", "Triangles\0"};
        static int primitive_type_current = 1;
        const char *comboLabel = types[primitive_type_current];
        static int primitive_type = GL_TRIANGLES;

        bool isRendered = m_registry.hasComponent<Engine::RenderComponent>(selectedEntity);
        ImGui::SameLine();
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

        if (auto material{m_registry.getComponent<Engine::OpenGLMaterialComponent>(m_currentEntity)})
        {
            ImGui::Text("Shader Material");
            ImGui::SameLine();
            ImGui::Button("Drag##OpenGLMaterial");
            createImGuiComponentDragSource<Engine::OpenGLMaterialComponent>(material);

            std::vector<Engine::MaterialUniformData> &materialsData{material->getMaterialData().second};

            for (Engine::MaterialUniformData &materialData : materialsData)
            {
                const char *name{std::get<0>(materialData).c_str()};
                unsigned int type{std::get<1>(materialData)};
                int offset{std::get<2>(materialData)};

                switch (type)
                {
                case GL_FLOAT:
                    ImGui::DragFloat(name, material->getProperty<float>(offset));
                    if (ImGui::IsItemEdited())
                    {
                        material->update();
                        m_registry.updated<Engine::OpenGLMaterialComponent>(selectedEntity);
                    }
                    break;
                case GL_FLOAT_VEC4:
                    ImGui::ColorEdit4(name, material->getProperty<float>(offset));
                    if (ImGui::IsItemEdited())
                    {
                        material->update();
                        m_registry.updated<Engine::OpenGLMaterialComponent>(selectedEntity);
                    }
                    break;
                case GL_FLOAT_VEC3:
                    ImGui::ColorEdit3(name, material->getProperty<float>(offset));
                    if (ImGui::IsItemEdited())
                    {
                        material->update();
                        m_registry.updated<Engine::OpenGLMaterialComponent>(selectedEntity);
                    }
                    break;
                }
            }
        }

        if (auto texture{m_registry.getComponent<Engine::OpenGLTextureComponent>(m_currentEntity)})
        {
            ImGui::Text("Texture");
            ImGui::SameLine();
            ImGui::Button("Drag##OpenGLTexture");
            createImGuiComponentDragSource<Engine::OpenGLTextureComponent>(texture);

            auto &textures{texture->getTextures()};

            int size = ImGui::GetWindowContentRegionMin().x + ImGui::GetWindowContentRegionMax().x;
            size = std::min(128, size);

            int index{0};
            for (auto &data : textures)
            {
                ImGui::Image((void *)data.getTexture(), ImVec2{(float)size, (float)size}, ImVec2{0, 1}, ImVec2{1, 0});

                if (auto path = createImGuiHighlightedDropTarget<fs::path>(
                        "system_path_payload",
                        [](const fs::path &path) { return (fs::is_regular_file(path) && isImage(path)); }))
                {
                    auto test = m_textureIndex.needTexture(*path, GL_TEXTURE_2D);
                    texture->editTexture(index, test);
                }

                ++index;
            }

            auto context{ImGui::GetCurrentContext()};
            auto payload{context->DragDropPayload};

            if (context->DragDropActive && payload.IsDataType("system_path_payload"))
            {
                auto path{(fs::path *)payload.Data};

                if (isImage(*path))
                {
                    ImGui::ColorButton("##New Texture", {1, 1, 1, 1}, 0, {(float)size, (float)size});

                    if (createImGuiHighlightedDropTarget<fs::path>("system_path_payload",
                                                                   [](const fs::path &path) { return true; }))
                    {
                        texture->addTexture(m_textureIndex.needTexture(*path, GL_TEXTURE_2D));
                    }
                }
            }
        }
        ImGui::Separator();
    }

    if (auto raytracingMaterial = m_registry.getComponent<Engine::RaytracingMaterial>(m_selectedEntity))
    {
        ImGui::Text("Raytracing Material");
        ImGui::SameLine();
        if (ImGui::Button("x##Raytrace"))
        {
            m_registry.removeComponent<Engine::RaytracingMaterial>(m_currentEntity);
        }

        ImGui::ColorEdit4("Color##Raytrace", raytracingMaterial->getColor().raw());
        bool isReflective = raytracingMaterial->isReflective();
        ImGui::Checkbox("Reflective##Raytrace", &isReflective);
        if (ImGui::IsItemClicked(0))
        {
            if (!isReflective)
            {
                raytracingMaterial->makeReflective();
            }
            else
            {
                raytracingMaterial->makeUnreflective();
            }
        }
        ImGui::Separator();
    }
    else
    {
        if (ImGui::Button("Add Raytracing Material"))
        {
            m_registry.createComponent<Engine::RaytracingMaterial>(m_selectedEntity);
        }
    }
}