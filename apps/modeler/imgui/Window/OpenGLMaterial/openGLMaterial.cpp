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

namespace fs = std::filesystem;

extern bool dragging;

UICreation::MaterialComponentWindow::MaterialComponentWindow(int &currentEntity,
                                                             Engine::Registry &registry,
                                                             Engine::Util::OpenGLTextureIndex &textureIndex)
    : ComponentWindow{"Material", currentEntity, registry}, m_textureIndex{textureIndex}, m_shaderWindow{currentEntity,
                                                                                                         registry}
{
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

void UICreation::MaterialComponentWindow::main()
{

    if (auto shader{m_registry.getComponent<Engine::OpenGLShaderComponent>(m_currentEntity)})
    {
        ImGui::Text("OpenGL Shader");
        ImGui::SameLine();
        ImGui::Button("Drag##OpenGLShader");
        createImGuiComponentDragSource<Engine::OpenGLShaderComponent>(shader);
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
            m_shaderWindow.open();
        }
        m_shaderWindow.render();

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

        ImGui::ColorEdit4("Color##Raytrace", raytracingMaterial->getColor().data());
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