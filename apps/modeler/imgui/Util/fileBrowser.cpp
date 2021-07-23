#include "fileBrowser.h"

#include <glad/glad.h>
#include <imgui.h>
#include <iostream>
#include <misc/cpp/imgui_stdlib.h>
#include <vector>

#include <OpenGL/Util/textureLoader.h>

using imageData = Engine::Util::textureInfo;

imageData directoryIcon;
imageData fileIcon;

void UIUtil::initFileBrowserIcons()
{
    directoryIcon = Engine::Util::loadTexture("../../data/icons/directory-icon.png", GL_TEXTURE_2D, GL_RGBA);
    fileIcon = Engine::Util::loadTexture("../../data/icons/file-icon.png", GL_TEXTURE_2D, GL_RGBA);
}

std::function<bool(const fs::path &)> UIUtil::can_open_function{[](const fs::path &) -> bool { return false; }};
std::function<void(const fs::path &, const std::string &)> UIUtil::open_function{
    [](const fs::path &, const std::string &) {}};

fs::path currentDirectory{fs::current_path().parent_path()};
std::string fileName{};
std::vector<fs::path> currentDirectoryContent;
bool initialized{false};
int currentSelection{-1};

bool visible{0};

void changeDirectory(const fs::path path)
{
    if (fs::is_directory(path))
    {
        currentDirectory = path;
        fileName = {};
        currentSelection = -1;
        currentDirectoryContent.resize(0);
        for (const auto &entry : std::filesystem::directory_iterator(path.c_str()))
        {
            if (entry.is_directory() || entry.is_regular_file())
            {
                // don't show hidden files
                if (entry.path().filename().c_str()[0] != '.')
                {
                    currentDirectoryContent.push_back(entry.path());
                }
            }
        }
    }
}

void addDirectory(const fs::path &path)
{
    if (!path.has_extension())
    {
        fs::create_directories(path);
        changeDirectory(currentDirectory);
    }
}

void resetFileFunctions()
{
    UIUtil::can_open_function = [](const fs::path &) -> bool { return false; };
    UIUtil::open_function = [](const fs::path &, const std::string &) {};
}

unsigned int getIcon(const fs::path &path)
{
    if (fs::is_directory(path))
    {
        return directoryIcon.buffer;
    }

    return fileIcon.buffer;
}

void drawDirectoryEntries()
{
    ImVec2 iconSize{64.0, 64.0};
    ImGuiStyle &style{ImGui::GetStyle()};
    int entryCount{currentDirectoryContent.size()};
    float window_visible_x2{ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x};

    for (int i{0}; i < entryCount; ++i)
    {
        const fs::path &entry = currentDirectoryContent[i];

        unsigned int buffer{getIcon(entry)};

        ImGui::PushID(i);

        ImGui::BeginGroup();
        ImGui::ImageButton(((void *)buffer), iconSize, ImVec2{0, 1}, ImVec2{1, 0});
        ImGui::Text(entry.filename().c_str());
        ImGui::EndGroup();

        if (ImGui::IsItemClicked(0))
        {
            currentSelection = i;
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
        {
            if (fs::is_directory(entry))
            {
                ImGui::PopID();
                changeDirectory(entry);
                return;
            }
            else if (UIUtil::can_open_function(entry))
            {
                UIUtil::open_function(entry, fileName);
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

void UIUtil::openFileBrowser() { visible = 1; }

void UIUtil::drawFileBrowser()
{
    if (!initialized)
    {
        changeDirectory(currentDirectory);
        initialized = true;
    }

    if (!visible)
    {
        return;
    }

    if (ImGui::Begin("File Browser", &visible))
    {
        ImGui::Text(currentDirectory.c_str());

        if (ImGui::Button("..##goto_parent_directory"))
        {
            currentSelection = -1;

            changeDirectory(currentDirectory.parent_path());
        }
        ImGui::SameLine();
        ImGui::InputText("##file_name", &fileName);
        // Allow adding of new directory if there is some input and it doesn't contain a dot
        if (fileName.size() && (fileName.find('.') == std::string::npos))
        {
            ImGui::SameLine();
            if (ImGui::Button("Add Directory"))
            {
                addDirectory(currentDirectory / fileName);
            }
        }

        drawDirectoryEntries();

        if (ImGui::Button("Close", ImVec2(120, 0)))
        {
            visible = 0;
        }
        bool openable{false};

        if (currentSelection < 0)
        {
            openable = can_open_function(currentDirectory);
        }
        else
        {
            openable = can_open_function(currentDirectoryContent[currentSelection]);
        }

        if (!openable)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        }
        ImGui::SameLine();
        if (ImGui::Button("Open", ImVec2(120, 0)))
        {
            if (openable)
            {
                if (currentSelection < 0)
                {
                    open_function(currentDirectory, fileName);
                }
                else
                {
                    open_function(currentDirectoryContent[currentSelection], fileName);
                }
                currentSelection = -1;
                resetFileFunctions();
                visible = 0;
            }
        }

        if (!openable)
        {
            ImGui::PopStyleVar();
        }

        ImGui::End();
    }
}