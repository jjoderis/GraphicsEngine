#include "mainViewPort.h"

#include "../../Util/SceneLoading/sceneLoader.h"
#include "../../Util/objectLoader.h"
#include "../helpers.h"
#include <Components/Camera/camera.h>
#include <Components/Tag/tag.h>
#include <Components/Transform/transform.h>
#include <Core/ECS/registry.h>
#include <Core/Util/Raycaster/raycaster.h>
#include <OpenGL/Renderer/renderer.h>
#include <filesystem>
#include <imgui.h>

namespace fs = std::filesystem;

UICreation::MainViewPort::MainViewPort(Engine::Registry &registry,
                                       Engine::OpenGLRenderer &renderer,
                                       int &selectedEntity,
                                       Engine::Util::OpenGLTextureIndex &textureIndex)
    : ImGuiWindow{"Main Viewport"}, m_registry{registry}, m_textureIndex{textureIndex},
      m_selectedEntity{selectedEntity}, m_renderer{renderer}, m_renderTracker{m_registry, m_renderables},
      m_framebuffer{
          {{GL_RGB, GL_RGB, GL_UNSIGNED_BYTE}, {GL_R16I, GL_RED_INTEGER, GL_INT}, {GL_RGB32F, GL_RGB, GL_FLOAT}}}
{
    m_framebuffer.setClearColorI({-1, -1, -1, -1}, 1);
    m_cameraEntity = registry.addEntity();
    registry.createComponent<Engine::TagComponent>(m_cameraEntity, "Modeler Camera");
    auto transform = registry.createComponent<Engine::TransformComponent>(m_cameraEntity);
    transform->setRotation(M_PI, {0.0, 1.0, 0.0});
    transform->update();
    m_camera = registry.createComponent<Engine::CameraComponent>(m_cameraEntity, registry);
    registry.updated<Engine::CameraComponent>(m_cameraEntity);

    m_cameraChangeCallback = registry.onAdded<Engine::ActiveCameraComponent>(
        [this, &registry](unsigned int entity, std::weak_ptr<Engine::ActiveCameraComponent> aC)
        {
            this->m_cameraEntity = entity;
            this->m_camera = registry.getComponent<Engine::CameraComponent>(entity);
        });
}

void UICreation::MainViewPort::main()
{
    m_framebuffer.clear();
    m_framebuffer.bind();
    m_renderer.render(m_renderables);
    m_framebuffer.unbind();

    auto size = ImGui::GetContentRegionAvail();
    ImGui::InvisibleButton(
        "main viewport", {size.x, size.y}, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

    if (auto path = UICreation::createImGuiHighlightedDropTarget<fs::path>(
            "system_path_payload",
            [](const fs::path &path)
            { return (fs::is_regular_file(path) && (path.extension() == ".gltf" || path.extension() == ".obj")); }))
    {
        if (path->extension() == ".gltf")
        {
            // m_registry.clear();
            Engine::Util::loadScene(*path, m_registry, m_textureIndex);
            m_camera->updateAspect((float)m_size(0) / (float)m_size(1));
            m_registry.updated<Engine::CameraComponent>(m_cameraEntity);
        }
        else if (path->extension() == ".obj")
        {
            Util::loadOBJFile(m_registry, *path, m_textureIndex);
        }
    }

    if (ImGui::IsItemClicked(ImGuiPopupFlags_MouseButtonLeft) || ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        m_grabbedEntity = -1;

        Engine::Math::IVector2 mousePos{ImGui::GetMousePos().x, ImGui::GetMousePos().y};

        Engine::Math::IVector2 pixelPosition{mousePos - m_pos};

        int index{};
        m_framebuffer.getPixel(
            &index, pixelPosition(0), m_size(1) - pixelPosition(1), GL_RED_INTEGER, GL_INT, GL_COLOR_ATTACHMENT1);

        if (index > -1)
        {
            m_selectedEntity = index;
            m_grabbedEntity = m_selectedEntity;
            m_framebuffer.getPixel(m_currentPoint.raw(),
                                   pixelPosition(0),
                                   m_size(1) - pixelPosition(1),
                                   GL_RGB,
                                   GL_FLOAT,
                                   GL_COLOR_ATTACHMENT2);
        }
        else if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            unsigned int activeCameraEntity = m_registry.getOwners<Engine::ActiveCameraComponent>()[0].front();
            auto camera = m_registry.getComponent<Engine::CameraComponent>(activeCameraEntity);
            Engine::Util::Ray cameraRay = camera->getCameraRay(pixelPosition, m_size);

            Engine::Math::Vector3 direction =
                camera->getViewMatrixInverse() * Engine::Math::Vector4{cameraRay.getDirection(), 0.0};
            m_currentPoint = (direction / direction.at(2)) * camera->getFar();
        }
        m_currentPixel = pixelPosition;
    }

    if (ImGui::IsItemActive())
    {
        auto mouseDelta = ImGui::GetIO().MouseDelta;
        auto scrolling = ImGui::GetIO().MouseWheel;

        if (mouseDelta.x || mouseDelta.y)
        {
            auto newPixel = m_currentPixel + Engine::Math::IVector2{mouseDelta.x, mouseDelta.y};
            auto invertXPixel = m_currentPixel + Engine::Math::IVector2{-mouseDelta.x, mouseDelta.y};

            if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_grabbedEntity > -1)
            {
                dragEntity(newPixel);
            }
            else if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
            {
                dragCamera(newPixel);
            }
            m_currentPixel = newPixel;
        }

        if (scrolling)
        {
            Engine::Math::Vector3 cameraSpacePosition =
                m_camera->getViewMatrix() * Engine::Math::Vector4{m_currentPoint, 1};
            cameraSpacePosition.normalize();
            auto cameraSpaceDirection = cameraSpacePosition * scrolling * 0.1;
            Engine::Math::Vector3 direction =
                m_camera->getViewMatrixInverse() * Engine::Math::Vector4{cameraSpaceDirection, 0};

            auto translation = m_registry.getComponent<Engine::TransformComponent>(m_selectedEntity);

            translation->translate(direction);
            translation->update();
            m_registry.updated<Engine::TransformComponent>(m_selectedEntity);

            m_currentPoint += direction;
        }

        if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_grabbedEntity < 0 &&
            ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            bool p{true};

            ImGui::SetDragDropPayload("scene_payload", &p, sizeof(bool));

            ImGui::Text("Scene");
            ImGui::EndDragDropSource();
        }
    }

    ImGui::GetWindowDrawList()->AddImage(
        (void *)m_framebuffer.getTexture(), ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), {0, 1}, {1, 0});
}

void UICreation::MainViewPort::dragEntity(const Engine::Math::IVector2 &newPixel)
{
    auto newRay = m_camera->getCameraSpaceRay(newPixel, m_size);

    Engine::Math::Vector3 cameraSpacePosition = m_camera->getViewMatrix() * Engine::Math::Vector4{m_currentPoint, 1};

    auto newCameraSpacePosition = (newRay.getDirection() / newRay.getDirection().at(2)) * cameraSpacePosition.at(2);

    auto t = newCameraSpacePosition - cameraSpacePosition;

    t = m_camera->getViewMatrixInverse() * Engine::Math::Vector4{t, 0};

    if (ImGui::IsKeyDown(82))
    {
        std::cout << "Not implemented\n";
    }
    else
    {
        auto transform = m_registry.getComponent<Engine::TransformComponent>(m_selectedEntity);

        transform->translate(t);
        transform->update();
        m_registry.updated<Engine::TransformComponent>(m_selectedEntity);

        m_currentPoint += t;
    }
}

void UICreation::MainViewPort::dragCamera(const Engine::Math::IVector2 &newPixel)
{
    auto oldRay{m_camera->getCameraSpaceRay(m_currentPixel, m_size)};
    auto newRay{m_camera->getCameraSpaceRay(newPixel, m_size)};
    if (ImGui::IsKeyDown(82))
    {
        float angle{acos(dot(oldRay.getDirection(), newRay.getDirection()))};
        auto axis{cross(oldRay.getDirection(), newRay.getDirection())};
        axis = m_camera->getViewMatrixInverse() * Engine::Math::Vector4{axis, 0};
        axis.normalize();

        auto cameraTransform{m_registry.getComponent<Engine::TransformComponent>(m_cameraEntity)};
        cameraTransform->rotate(angle, axis);
        m_registry.updated<Engine::TransformComponent>(m_cameraEntity);
    }
    else
    {
        Engine::Math::Vector3 cameraSpacePosition =
            m_camera->getViewMatrix() * Engine::Math::Vector4{m_currentPoint, 1};

        auto newCameraSpacePosition = (newRay.getDirection() / newRay.getDirection().at(2)) * cameraSpacePosition.at(2);

        auto t = newCameraSpacePosition - cameraSpacePosition;

        t = m_camera->getViewMatrixInverse() * Engine::Math::Vector4{t, 0};

        if (t.norm() > 200)
        {
            t.normalize();
            t *= 200;
        }

        auto transform = m_registry.getComponent<Engine::TransformComponent>(m_cameraEntity);
        transform->translate(-t);
        transform->update();
        m_registry.updated<Engine::TransformComponent>(m_cameraEntity);
    }
}

void UICreation::MainViewPort::onResize()
{
    ImGuiWindow::onResize();

    int width = m_size.at(0);
    int height = m_size.at(1);

    m_framebuffer.resize(width, height);

    std::vector<std::shared_ptr<Engine::CameraComponent>> cameras = m_registry.getComponents<Engine::CameraComponent>();

    for (std::shared_ptr<Engine::CameraComponent> &camera : cameras)
    {
        camera->updateAspect((float)width / (float)height);

        const std::list<unsigned int> owners{m_registry.getOwners<Engine::CameraComponent>(camera)};
        m_registry.updated<Engine::CameraComponent>(owners.front());
    }
}