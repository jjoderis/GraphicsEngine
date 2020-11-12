#include "glad/opengl.h"
#include "glfw/window.h"
#include "imgui/imgui.h"

#include <Math/math.h>
#include <ECS/registry.h>
#include <Core/Components/Material/material.h>
#include <Core/Components/Geometry/geometry.h>
#include <Core/Components/Transform/transform.h>
#include <OpenGL/Components/Render/render.h>

#include <iostream>

GLFWwindow* Window::m_window = nullptr;

int main()
{
    Window::init();
    OpenGL::init();
    UI::init();

    Engine::Registry registry{};

    // add callback that is invoked every time a RenderComponent is added to an entity which then associates them
    std::shared_ptr<std::function<void(unsigned int, Engine::OpenGLRenderComponent*)>> a = 
        registry.onAdded<Engine::OpenGLRenderComponent>(
            [](unsigned int entity, Engine::OpenGLRenderComponent* renderComponent) {
                renderComponent->associate(entity);
            }
        );

    GLFWwindow* window = Window::getWindow();

    unsigned int triangle1{registry.addEntity()};
    registry.addComponent<Engine::TagComponent>(triangle1, new Engine::TagComponent{"Triangle 1"});
    unsigned int triangle2{registry.addEntity()};
    registry.addComponent<Engine::TagComponent>(triangle2, new Engine::TagComponent{"Triangle 2"});

    while (!glfwWindowShouldClose(window))
    {
        Window::preRender();

        OpenGL::preRender();

        UI::preRender();

        UI::render(registry);

        const std::vector<std::unique_ptr<Engine::OpenGLRenderComponent>> &renderComponents = registry.getComponents<Engine::OpenGLRenderComponent>();

        for( const std::unique_ptr<Engine::OpenGLRenderComponent>& renderComponent: renderComponents ) {
            renderComponent.get()->render();
        }

        UI::postRender();

        Window::postRender();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}