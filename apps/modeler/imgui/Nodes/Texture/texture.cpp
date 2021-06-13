#include "texture.h"

#include "../../Util/errorModal.h"
#include "../../Util/fileBrowser.h"
#include <OpenGL/Components/Texture/texture.h>
#include <glad/glad.h>
#include <iostream>

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
        ImGui::Text(data.first.c_str());
        if (ImGui::IsItemClicked(0))
        {
            UIUtil::can_open_function = [](const fs::path &path) -> bool
            { return (fs::is_regular_file(path) && isImage(path)); };
            UIUtil::open_function = [&registry, texture, index](const fs::path &path, const std::string &fileName)
            {
                texture->editTexture(index, path, GL_TEXTURE_2D);
                registry.updated<Engine::OpenGLTextureComponent>(selectedEntity);
            };
            UIUtil::openFileBrowser();
        }
        ++index;
    }

    if (ImGui::Button("Add##new_texture"))
    {
        UIUtil::can_open_function = [](const fs::path &path) -> bool
        { return (fs::is_regular_file(path) && isImage(path)); };
        UIUtil::open_function = [&](const fs::path &path, const std::string &fileName)
        {
            texture->addTexture(path, GL_TEXTURE_2D);
            registry.updated<Engine::OpenGLTextureComponent>(selectedEntity);
        };
        UIUtil::openFileBrowser();
    }
}