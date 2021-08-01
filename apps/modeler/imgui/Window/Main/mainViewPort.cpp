#include "mainViewPort.h"

#include <OpenGL/Renderer/renderer.h>
#include <imgui.h>
#include <Components/Camera/camera.h>
#include <Core/ECS/registry.h>
#include <Components/Transform/transform.h>
#include <Core/Util/Raycaster/raycaster.h>


UICreation::MainViewPort::MainViewPort(Engine::Registry &registry, Engine::OpenGLRenderer &renderer, int &selectedEntity) 
  : ImGuiWindow{"Main Viewport"} ,m_registry{registry}, m_selectedEntity{selectedEntity}, m_renderer{renderer}, m_renderTracker{m_registry, m_renderables}
{
}

void UICreation::MainViewPort::main() {
  m_framebuffer.clear();
  m_framebuffer.bind();
  m_renderer.render(m_renderables);
  m_framebuffer.unbind();

  auto size = ImGui::GetContentRegionAvail();
  ImGui::InvisibleButton("main viewport", {size.x, size.y}, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
  if (ImGui::IsItemClicked(ImGuiPopupFlags_MouseButtonLeft)) {
    m_grabbedEntity = -1;

    auto viewportMin = ImGui::GetItemRectMin();
    auto viewportMax = ImGui::GetItemRectMax();
    auto mousePos = ImGui::GetMousePos();  

    Engine::Math::IVector2 pixelPosition{mousePos.x - viewportMin.x, mousePos.y - viewportMin.y};
    Engine::Math::IVector2 viewportSize{viewportMax.x - viewportMin.x, viewportMax.y - viewportMin.y};

    unsigned int activeCameraEntity = m_registry.getOwners<Engine::ActiveCameraComponent>()[0].front();
    auto camera = m_registry.getComponent<Engine::CameraComponent>(activeCameraEntity);
    Engine::Util::Ray cameraRay = camera->getCameraRay(pixelPosition, viewportSize);

    auto intersections = Engine::Util::castRay(cameraRay, m_registry);

    if (intersections.size())
    {
        m_selectedEntity = intersections.begin()->getEntity();
        m_grabbedEntity = m_selectedEntity;
        m_currentPoint = intersections.begin()->getIntersection();
        m_currentPixel = pixelPosition;
    }
  }

  if (ImGui::IsItemActive() && m_grabbedEntity > -1) {
    auto mouseDelta = ImGui::GetIO().MouseDelta;

    if (mouseDelta.x || mouseDelta.y) {
      unsigned int activeCameraEntity = m_registry.getOwners<Engine::ActiveCameraComponent>()[0].front();
      auto camera = m_registry.getComponent<Engine::CameraComponent>(activeCameraEntity);

      auto viewportMin = ImGui::GetItemRectMin();
      auto viewportMax = ImGui::GetItemRectMax();
      Engine::Math::IVector2 viewportSize{viewportMax.x - viewportMin.x, viewportMax.y - viewportMin.y};

      auto newPixel = m_currentPixel + Engine::Math::IVector2{mouseDelta.x, mouseDelta.y};
      auto newRay = camera->getCameraRay(newPixel, viewportSize);

      newRay = camera->getViewMatrix() * newRay;

      Engine::Math::Vector3 cameraSpacePosition = camera->getViewMatrix() * Engine::Math::Vector4{m_currentPoint, 1};

      auto newCameraSpacePosition = (newRay.getDirection() / newRay.getDirection().at(2)) * cameraSpacePosition.at(2);

      auto t = newCameraSpacePosition - cameraSpacePosition;

      t = camera->getViewMatrix() * Engine::Math::Vector4{t, 1};

      auto translation = m_registry.getComponent<Engine::TransformComponent>(m_selectedEntity);

      translation->translate(t);
      translation->update();
      m_registry.updated<Engine::TransformComponent>(m_selectedEntity);

      m_currentPoint += t;
      m_currentPixel = newPixel;
    }
  }
        
  ImGui::GetWindowDrawList()->AddImage((void *)m_framebuffer.getTexture(), ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), {0, 1}, {1, 0});
}

void UICreation::MainViewPort::onResize() {
  ImGuiWindow::onResize();

  m_framebuffer.resize(m_width, m_height);

  std::vector<std::shared_ptr<Engine::CameraComponent>> cameras = m_registry.getComponents<Engine::CameraComponent>();

  for (std::shared_ptr<Engine::CameraComponent> &camera : cameras)
  {
      camera->updateAspect((float)m_width / (float)m_height);

      const std::list<unsigned int> owners{m_registry.getOwners<Engine::CameraComponent>(camera)};
      m_registry.updated<Engine::CameraComponent>(owners.front());
  }
}