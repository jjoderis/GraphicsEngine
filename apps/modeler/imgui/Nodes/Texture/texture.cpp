#include "texture.h"

#include "../../Util/errorModal.h"
#include "../../Util/fileBrowser.h"
#include <OpenGL/Components/Texture/texture.h>
#include <OpenGL/Util/textureIndex.h>
#include <glad/glad.h>
#include <iostream>

extern bool dragging;

UICreation::TextureComponentWindow::TextureComponentWindow(int &currentEntity, Engine::Registry &registry, Engine::Util::OpenGLTextureIndex &textureIndex)
    : TemplatedComponentWindow<Engine::OpenGLTextureComponent>{"Texture", currentEntity, registry}, m_textureIndex{textureIndex}
{    
}

bool isImage(const fs::path &path)
{
    fs::path extension{path.extension().c_str()};

    return extension == ".jpg" || extension == ".jpeg" || extension == ".png";
}

void UICreation::TextureComponentWindow::main() {
    createImGuiComponentDragSource<Engine::OpenGLTextureComponent>(dragging);

    auto textures{m_component->getTextures()};

    int index{0};
    for (auto data : textures)
    {
        int size = ImGui::GetWindowContentRegionMin().x + ImGui::GetWindowContentRegionMax().x;
        size = std::min(128, size);
        ImGui::Image((void *)data.getTexture(), ImVec2{(float)size, (float)size}, ImVec2{0, 1}, ImVec2{1, 0});
        if (ImGui::IsItemClicked(0))
        {
            UIUtil::can_open_function = [](const fs::path &path) -> bool
            { return (fs::is_regular_file(path) && isImage(path)); };
            UIUtil::open_function = [this, index](const fs::path &path, const std::string &fileName)
            { m_component->editTexture(index, m_textureIndex.needTexture(path, GL_TEXTURE_2D)); };
            UIUtil::openFileBrowser();
        }
        ++index;
    }

    if (ImGui::Button("Add##new_texture"))
    {
        UIUtil::can_open_function = [](const fs::path &path) -> bool
        { return (fs::is_regular_file(path) && isImage(path)); };
        UIUtil::open_function = [this](const fs::path &path, const std::string &fileName)
        { m_component->addTexture(m_textureIndex.needTexture(path, GL_TEXTURE_2D)); };
        UIUtil::openFileBrowser();
    }
}