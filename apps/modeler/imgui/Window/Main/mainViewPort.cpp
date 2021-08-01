#include "mainViewPort.h"

#include <OpenGL/Renderer/renderer.h>
#include <imgui.h>
#include <Components/Camera/camera.h>
#include <Core/ECS/registry.h>
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