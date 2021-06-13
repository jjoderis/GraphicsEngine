#include "texture.h"

#include "../../Util/errorModal.h"
#include "../../Util/fileBrowser.h"
#include <OpenGL/Components/Texture/texture.h>
#include <glad/glad.h>
#include <iostream>

extern bool dragging;

template <>
void UICreation::createComponentNodeMain<Engine::OpenGLTextureComponent>(
    std::shared_ptr<Engine::OpenGLTextureComponent> texture, Engine::Registry &registry)
{
    createImGuiComponentDragSource<Engine::OpenGLTextureComponent>(dragging);

    auto textures{texture->getTextures()};

    for (auto data : textures)
    {
        ImGui::Text(data.first.c_str());
    }

    if (ImGui::Button("Add##new_texture"))
    {
        UIUtil::can_open_function = [](const fs::path &path) -> bool
        { return (fs::is_regular_file(path) && path.extension() == ".jpg"); };
        UIUtil::open_function = [&](const fs::path &path, const std::string &fileName)
        {
            texture->addTexture(path, GL_TEXTURE_2D);
            registry.updated<Engine::OpenGLTextureComponent>(selectedEntity);
        };
        ImGui::OpenPopup("File Browser");
    }

    UIUtil::drawFileBrowser();
}

// template <>
// void UICreation::createComponentNodeMain<Engine::TransformComponent>(
//     std::shared_ptr<Engine::TransformComponent> transform, Engine::Registry &registry)
// {
//     createImGuiComponentDragSource<Engine::TransformComponent>(dragging);

//     ImGui::DragFloat3("Translation", transform->getTranslation().raw(), 0.1);
//     if (ImGui::IsItemEdited())
//     {
//         transform->update();
//         registry.updated<Engine::TransformComponent>(selectedEntity);
//     }
//     ImGui::DragFloat3("Scaling", transform->getScaling().raw(), 0.1);
//     if (ImGui::IsItemEdited())
//     {
//         transform->update();
//         registry.updated<Engine::TransformComponent>(selectedEntity);
//     }

//     auto rotDeg = MathLib::Util::radToDeg(transform->getRotation());
//     ImGui::DragFloat3("Rotation", rotDeg.raw(), 1.0);
//     if (ImGui::IsItemEdited())
//     {
//         transform->setRotation(MathLib::Util::degToRad(rotDeg));
//         transform->update();
//         registry.updated<Engine::TransformComponent>(selectedEntity);
//     }
// }

// void UICreation::drawTransformNode(Engine::Registry &registry)
// {
//     if (std::shared_ptr<Engine::TransformComponent> transform =
//             registry.getComponent<Engine::TransformComponent>(selectedEntity))
//     {
//         createComponentNodeOutline<Engine::TransformComponent>(
//             "Transform",
//             registry,
//             transform.get(),
//             [&]()
//             {
//
//             });
//     }
// }