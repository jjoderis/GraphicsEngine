#ifndef APPS_MODELER_IMGUI_WINDOW_FILE_BROWSER
#define APPS_MODELER_IMGUI_WINDOW_FILE_BROWSER

#include "../Templates/imguiWindow.h"

#include <OpenGL/Util/textureIndex.h>
#include <filesystem>
#include <vector>

namespace Engine
{
class Registry;
}

namespace UICreation
{
/**
 * A content browser that allows for import and export of assets by dragging and dropping
 *
 * Idea was first seen here: https://www.youtube.com/watch?v=yjVSs1HtAng
 **/
class FileBrowser : public ImGuiWindow
{
public:
    FileBrowser() = delete;
    FileBrowser(Engine::Registry &registry, Engine::Util::OpenGLTextureIndex &textureIndex);

private:
    Engine::Util::OpenGLTextureIndex &m_textureIndex;
    Engine::Registry &m_registry;

    std::filesystem::path m_currentPath{std::filesystem::current_path()};
    std::vector<std::filesystem::path> m_directoryContents{};
    std::string m_fileInput{};

    Engine::Util::OpenGLTextureHandler m_directoryIcon;
    Engine::Util::OpenGLTextureHandler m_fileIcon;

    virtual void main() override;

    void changeDirectory(const char *subPath);
    void loadDirectoryContent();
    void addDirectory(const std::filesystem::path &path);

    unsigned int getIcon(const std::filesystem::path &path);

    void drawEntries();
};

} // namespace UICreation

#endif