#include "fileBrowser.h"

#include "../../Util/SceneLoading/sceneLoader.h"
#include "../helpers.h"
#include <OpenGL/Shader/shader.h>
#include <algorithm>
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

UICreation::FileBrowser::FileBrowser(Engine::Registry &registry, Engine::Util::OpenGLTextureIndex &textureIndex)
    : ImGuiWindow{"File Browser"}, m_registry{registry}, m_textureIndex{textureIndex},
      m_directoryIcon{textureIndex.needTexture("../../data/icons/directory-icon.png", GL_TEXTURE_2D, GL_RGBA)},
      m_fileIcon{textureIndex.needTexture("../../data/icons/file-icon.png", GL_TEXTURE_2D, GL_RGBA)}
{
    loadDirectoryContent();
}

void UICreation::FileBrowser::changeDirectory(const char *subPath)
{
    m_currentPath /= subPath;
    m_currentPath = std::filesystem::canonical(m_currentPath);
    m_fileInput.clear();

    loadDirectoryContent();
}

void UICreation::FileBrowser::loadDirectoryContent()
{
    m_directoryContents.clear();

    for (const auto &entry : std::filesystem::directory_iterator(m_currentPath.c_str()))
    {
        if (entry.is_directory() || entry.is_regular_file())
        {
            // don't show hidden files
            if (entry.path().filename().c_str()[0] != '.')
            {
                m_directoryContents.push_back(entry.path());
            }
        }
    }

    // sort directory contents in lexicographic order
    std::sort(m_directoryContents.begin(),
              m_directoryContents.end(),
              [](std::filesystem::path a, std::filesystem::path b)
              {
                  auto aString{a.filename().string()};
                  auto bString{b.filename().string()};

                  for (int i{0}; i < aString.size(); ++i)
                  {
                      auto lowerA{tolower(aString[i])};
                      auto lowerB{tolower(bString[i])};
                      if (lowerA != lowerB)
                      {
                          return lowerA < lowerB;
                      }
                  }

                  return true;
              });
}

void UICreation::FileBrowser::main()
{
    ImGui::BeginGroup();

    ImGui::Text(m_currentPath.c_str());

    if (ImGui::Button("..##goto_parent_directory"))
    {
        changeDirectory("..");
    }

    ImGui::SameLine();
    ImGui::InputText("##file_name", &m_fileInput);
    // Allow adding of new directory if there is some input and it doesn't contain a dot
    if (m_fileInput.size() && (m_fileInput.find('.') == std::string::npos))
    {
        ImGui::SameLine();
        if (ImGui::Button("Add Directory"))
        {
            addDirectory(m_currentPath / m_fileInput);
        }
    }

    drawEntries();

    ImGui::EndGroup();

    if (createImGuiHighlightedDropTarget<bool>("scene_payload", [this](const bool &a) { return true; }))
    {
        Engine::Util::saveScene(m_currentPath, m_registry, m_textureIndex);
        this->changeDirectory(".");
    }
}

unsigned int UICreation::FileBrowser::getIcon(const std::filesystem::path &path)
{
    if (std::filesystem::is_directory(path))
    {
        return m_directoryIcon.getTexture();
    }

    return m_fileIcon.getTexture();
}

void UICreation::FileBrowser::drawEntries()
{
    ImVec2 iconSize{64.0, 64.0};
    auto &style{ImGui::GetStyle()};
    int entryCount{m_directoryContents.size()};
    auto window_visible_x2{ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x};

    for (int i{0}; i < entryCount; ++i)
    {
        auto &entry{m_directoryContents[i]};

        unsigned int buffer{getIcon(entry)};

        ImGui::PushID(i);

        ImGui::BeginGroup();
        ImGui::ImageButton(((void *)buffer), iconSize, ImVec2{0, 1}, ImVec2{1, 0});

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("system_path_payload", &entry, sizeof(std::filesystem::path));

            ImGui::Text(entry.c_str());
            ImGui::EndDragDropSource();
        }

        if (std::filesystem::is_directory(entry))
        {
            if (createImGuiHighlightedDropTarget<bool>("scene_payload", [this](const bool &a) { return true; }))
            {
                Engine::Util::saveScene(entry, m_registry, m_textureIndex);
            }

            if (auto shaders = createImGuiHighlightedDropTarget<std::vector<Engine::OpenGLShader>>(
                    "shaders_payload", [](const std::vector<Engine::OpenGLShader> &shaders) { return true; }))
            {
                Engine::saveShaders(entry, *shaders);
            }
        }

        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + iconSize.x);
        ImGui::TextUnformatted(entry.filename().c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndGroup();

        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            if (std::filesystem::is_directory(entry))
            {
                ImGui::PopID();
                changeDirectory(entry.filename().c_str());
                return;
            }
        }

        float lastButtonX2{ImGui::GetItemRectMax().x};
        float nextButtonX2{lastButtonX2 + style.ItemSpacing.x + iconSize.x};

        if (i + 1 < entryCount && nextButtonX2 < window_visible_x2)
        {
            ImGui::SameLine();
        }

        ImGui::PopID();
    }
}

void UICreation::FileBrowser::addDirectory(const std::filesystem::path &path)
{
    if (!path.has_extension())
    {
        std::filesystem::create_directories(path);
        changeDirectory(path.filename().c_str());
    }
}