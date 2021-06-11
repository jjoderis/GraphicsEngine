#include "fileBrowser.h"

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include <vector>

std::function<bool(const fs::path &)> UIUtil::can_open_function{[](const fs::path &) -> bool { return false; }};
std::function<void(const fs::path &, const std::string &)> UIUtil::open_function{
    [](const fs::path &, const std::string &) {}};

fs::path currentDirectory{fs::current_path().parent_path()};
std::string fileName{};
std::vector<fs::path> currentDirectoryContent;
bool initialized{false};
int currentSelection{-1};

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
                currentDirectoryContent.push_back(entry.path());
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

void UIUtil::drawFileBrowser()
{
    if (!initialized)
    {
        changeDirectory(currentDirectory);
        initialized = true;
    }

    ImGui::SetNextWindowSize(ImVec2(550, 680));

    if (ImGui::BeginPopupModal("File Browser", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
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

        for (int i{0}; i < currentDirectoryContent.size(); ++i)
        {
            const fs::path &entry = currentDirectoryContent[i];

            // doesn't work for some reason
            if (ImGui::Selectable(entry.c_str(),
                                  i == currentSelection,
                                  ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick))
            {
                currentSelection = i;
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    if (fs::is_directory(entry))
                    {
                        changeDirectory(entry);
                    }
                    else if (can_open_function(entry))
                    {
                        open_function(entry, fileName);
                    }
                }
            }
        }
        if (ImGui::Selectable(
                "..", false, ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_AllowDoubleClick))
        {
            currentSelection = -1;
            if (ImGui::IsMouseDoubleClicked(0))
            {
                changeDirectory(currentDirectory.parent_path());
            }
        }
        if (ImGui::Button("Close", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
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
                ImGui::CloseCurrentPopup();
            }
        }

        if (!openable)
        {
            ImGui::PopStyleVar();
        }

        ImGui::EndPopup();
    }
}