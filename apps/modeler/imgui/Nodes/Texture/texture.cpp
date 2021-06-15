#include "texture.h"

#include "../../Util/errorModal.h"
#include "../../Util/fileBrowser.h"
#include <OpenGL/Components/Texture/texture.h>
#include <OpenGL/Util/textureIndex.h>
#include <glad/glad.h>
#include <iostream>

extern Engine::Util::OpenGLTextureIndex textureIndex;

extern bool dragging;

bool isImage(const fs::path &path)
{
    fs::path extension{path.extension().c_str()};

    return extension == ".jpg" || extension == ".jpeg" || extension == ".png";
}

template <>
void UICreation::createComponentNodeMain<Engine::OpenGLTextureComponent>(
    std::shared_ptr<Engine::OpenGLTextureComponent> texture, Engine::Registry &registry)
{
    createImGuiComponentDragSource<Engine::OpenGLTextureComponent>(dragging);

    auto textures{texture->getTextures()};

    int index{0};
    for (auto data : textures)
    {
        int size = ImGui::GetWindowContentRegionMin().x + ImGui::GetWindowContentRegionMax().x;
        size = std::min(128, size);
        ImGui::Image((void *)data.first, ImVec2{(float)size, (float)size}, ImVec2{0, 1}, ImVec2{1, 0});
        if (ImGui::IsItemClicked(0))
        {
            UIUtil::can_open_function = [](const fs::path &path) -> bool
            { return (fs::is_regular_file(path) && isImage(path)); };
            UIUtil::open_function = [&registry, texture, index](const fs::path &path, const std::string &fileName)
            {
                unsigned int oldBuffer{texture->getTexture(index).first};
                if (oldBuffer)
                {
                    textureIndex.unneedTexture(oldBuffer, texture.get());
                }
                texture->editTexture(
                    index, textureIndex.needTexture(path, GL_TEXTURE_2D, texture.get()), GL_TEXTURE_2D);
            };
            UIUtil::openFileBrowser();
        }
        ++index;
    }

    if (ImGui::Button("Add##new_texture"))
    {
        UIUtil::can_open_function = [](const fs::path &path) -> bool
        { return (fs::is_regular_file(path) && isImage(path)); };
        UIUtil::open_function = [texture](const fs::path &path, const std::string &fileName)
        {
            // TODO: texture type
            texture->addTexture(textureIndex.needTexture(path, GL_TEXTURE_2D, texture.get()), GL_TEXTURE_2D);
        };
        UIUtil::openFileBrowser();
    }
}