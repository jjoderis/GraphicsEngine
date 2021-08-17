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
    : ImGuiWindow{"Main Viewport"}, m_registry{registry}, m_textureIndex{textureIndex}, m_postProcesser{registry,
                                                                                                        selectedEntity},
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
    // render the scene into a separate frame buffer
    m_framebuffer.clear();
    m_framebuffer.bind();
    m_renderer.render(m_renderables);
    m_framebuffer.unbind();

    // create invisible button that spans the whole viewport to handle click events on it
    auto size = ImGui::GetContentRegionAvail();
    ImGui::InvisibleButton(
        "main viewport", {size.x, size.y}, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

    // allow objects to be imported when a gltf or obj file is dropped onto the main viewport
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
        onMouseClick();
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        m_grabbedEntity = -1;
    }

    if (ImGui::IsMouseDragging(ImGuiPopupFlags_MouseButtonLeft) || ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        auto mouseDelta = Engine::Math::IVector2{ImGui::GetIO().MouseDelta.x, ImGui::GetIO().MouseDelta.y};

        onMouseDrag(mouseDelta);
    }

    auto scrolling = ImGui::GetIO().MouseWheel;

    if (scrolling)
    {
        onMouseScroll(scrolling);
    }

    // allow export of scene (as gltf) by dragging into file browser
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && m_grabbedEntity < 0 &&
        ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        bool p{true};

        ImGui::SetDragDropPayload("scene_payload", &p, sizeof(bool));

        ImGui::Text("Scene");
        ImGui::EndDragDropSource();
    }

    m_postProcesser.postProcess(m_framebuffer.getTexture());

    ImGui::GetWindowDrawList()->AddImage((void *)m_postProcesser.getFramebuffer().getTexture(),
                                         ImGui::GetItemRectMin(),
                                         ImGui::GetItemRectMax(),
                                         {0, 1},
                                         {1, 0});
}

void UICreation::MainViewPort::onMouseClick()
{
    m_grabbedEntity = -1;
    m_clickedEntity = -1;

    Engine::Math::IVector2 mousePos{ImGui::GetMousePos().x, ImGui::GetMousePos().y};

    Engine::Math::IVector2 pixelPosition{mousePos - m_pos};

    int index{};
    m_framebuffer.getPixel(
        &index, pixelPosition(0), m_size(1) - pixelPosition(1), GL_RED_INTEGER, GL_INT, GL_COLOR_ATTACHMENT1);

    if (index > -1)
    {
        m_clickedEntity = index;
        m_framebuffer.getPixel(m_currentPoint.raw(),
                               pixelPosition(0),
                               m_size(1) - pixelPosition(1),
                               GL_RGB,
                               GL_FLOAT,
                               GL_COLOR_ATTACHMENT2);
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        onLeftClick(pixelPosition);
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        onRightClick(pixelPosition);
    }

    m_currentPixel = pixelPosition;
}
void UICreation::MainViewPort::onLeftClick(const Engine::Math::IVector2 &clickedPixel)
{
    m_grabbedEntity = m_clickedEntity;

    if (m_clickedEntity > -1)
    {
        m_selectedEntity = m_clickedEntity;
    }
}
void UICreation::MainViewPort::onRightClick(const Engine::Math::IVector2 &clickedPixel)
{
    unsigned int activeCameraEntity = m_registry.getOwners<Engine::ActiveCameraComponent>()[0].front();
    auto camera = m_registry.getComponent<Engine::CameraComponent>(activeCameraEntity);
    Engine::Util::Ray cameraRay = camera->getCameraRay(clickedPixel, m_size);

    Engine::Math::Vector3 direction =
        camera->getViewMatrixInverse() * Engine::Math::Vector4{cameraRay.getDirection(), 0.0};

    if (m_clickedEntity < -1)
    {
        m_currentPoint = (direction / direction.at(2)) * camera->getFar();
    }
}

void UICreation::MainViewPort::onMouseDrag(const Engine::Math::IVector2 &dragDelta)
{
    auto newPixel = m_currentPixel + dragDelta;

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

void UICreation::MainViewPort::onMouseScroll(float scroll)
{
    // just move the camera if nothing is grabbed
    if (m_grabbedEntity < 0)
    {
        auto direction{m_camera->getViewMatrixInverse() * (Engine::Math::Vector4{0, 0, -1, 0} * scroll * 0.1)};
        auto transform{m_registry.getComponent<Engine::TransformComponent>(m_cameraEntity)};
        transform->translate(direction);
        transform->update();
        m_registry.updated<Engine::TransformComponent>(m_cameraEntity);
    }
    // move the grabbed entity towards the camera
    else
    {
        Engine::Math::Vector3 cameraSpacePosition =
            m_camera->getViewMatrix() * Engine::Math::Vector4{m_currentPoint, 1};
        cameraSpacePosition.normalize();
        auto cameraSpaceDirection = cameraSpacePosition * scroll * 0.1;
        Engine::Math::Vector3 direction =
            m_camera->getViewMatrixInverse() * Engine::Math::Vector4{cameraSpaceDirection, 0};

        auto translation = m_registry.getComponent<Engine::TransformComponent>(m_selectedEntity);

        translation->translate(direction);
        translation->update();
        m_registry.updated<Engine::TransformComponent>(m_selectedEntity);

        m_currentPoint += direction;
    }
}

void UICreation::MainViewPort::onResize()
{
    ImGuiWindow::onResize();

    int width = m_size.at(0);
    int height = m_size.at(1);

    m_framebuffer.resize(width, height);
    m_postProcesser.resize(width, height);

    std::vector<std::shared_ptr<Engine::CameraComponent>> cameras = m_registry.getComponents<Engine::CameraComponent>();

    for (std::shared_ptr<Engine::CameraComponent> &camera : cameras)
    {
        camera->updateAspect((float)width / (float)height);

        const std::list<unsigned int> owners{m_registry.getOwners<Engine::CameraComponent>(camera)};
        m_registry.updated<Engine::CameraComponent>(owners.front());
    }
}