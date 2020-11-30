#include "glad/opengl.h"
#include "glfw/window.h"
#include "imgui/imgui.h"

#include <Math/math.h>
#include <ECS/registry.h>
#include <Core/Components/Material/material.h>
#include <Core/Components/Geometry/geometry.h>
#include <Core/Components/Transform/transform.h>
#include <OpenGL/Components/Render/render.h>
#include <OpenGL/Renderer/renderer.h>
#include <Core/Components/Light/light.h>
#include <Util/fileHandling.h>

#include <iostream>

GLFWwindow* Window::m_window = nullptr;
Engine::Registry registry{};

void cameraAspectCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

    std::vector<std::shared_ptr<Engine::CameraComponent>> cameras = registry.getComponents<Engine::CameraComponent>();

    for(std::shared_ptr<Engine::CameraComponent>& camera: cameras) {
        camera->updateAspect((float)width/(float)height);

        const std::list<unsigned int> owners{registry.getOwners<Engine::CameraComponent>(camera)};
        registry.updated<Engine::CameraComponent>(owners.front());
    }
}

int main()
{
    Window::init();
    OpenGL::init();

    Engine::OpenGLRenderer* renderer = new Engine::OpenGLRenderer{registry};

    // we have to delete the renderer before we tear down the OpenGL Context => we have to be able to delete the renderer before this function ends
    GLFWwindow* window = Window::getWindow();

    glfwSetFramebufferSizeCallback(window, cameraAspectCallback);

    UI::init(registry);

    unsigned int light1{registry.addEntity()};
    registry.addComponent<Engine::TagComponent>(light1, std::make_shared<Engine::TagComponent>("Light 1"));
    registry.addComponent<Engine::DirectionalLightComponent>(light1, std::make_shared<Engine::DirectionalLightComponent>());
    registry.addComponent<Engine::TransformComponent>(light1, std::make_shared<Engine::TransformComponent>());
    registry.addComponent<Engine::GeometryComponent>(light1, Engine::createSphereGeometry(0.1, 20, 20));
    registry.addComponent<Engine::MaterialComponent>(light1, std::make_shared<Engine::MaterialComponent>(1.0, 1.0, 1.0, 1.0));

    unsigned int object1{registry.addEntity()};
    registry.addComponent<Engine::TagComponent>(object1, std::make_shared<Engine::TagComponent>("Object 1"));
    registry.addComponent<Engine::MaterialComponent>(object1, std::make_shared<Engine::MaterialComponent>( 1, 0, 0, 1.0 ));
    std::weak_ptr<Engine::TransformComponent> transform{ registry.addComponent<Engine::TransformComponent>(object1, std::make_shared<Engine::TransformComponent>()) };
    transform.lock()->translate(Engine::Math::Vector3{ 0.0f, 0.0f, 4.0f });
    transform.lock()->update();
    std::weak_ptr<Engine::GeometryComponent> geometry{ registry.addComponent<Engine::GeometryComponent>(object1, Engine::createSphereGeometry(1.0, 20, 20)) };
    registry.addComponent<Engine::OpenGLRenderComponent>(object1, std::make_shared<Engine::OpenGLRenderComponent>(
        registry,
        Engine::loadShaders("../../data/shaders/Basic_Shading_Shader")
    ));

    while (!glfwWindowShouldClose(window))
    {
        Window::preRender();

        OpenGL::preRender();

        UI::preRender();

        UI::render(registry);

        renderer->render();

        UI::postRender();

        Window::postRender();
    }

    delete renderer;

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}