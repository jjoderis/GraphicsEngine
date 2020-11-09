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

    // TODO: add callback that is invoked every time a RenderComponent is added to an entity which then associates them
    std::shared_ptr<std::function<void(unsigned int, Engine::OpenGLRenderComponent*)>> a = 
        registry.onAdded<Engine::OpenGLRenderComponent>(
            [](unsigned int entity, Engine::OpenGLRenderComponent* renderComponent) {
                renderComponent->associate(entity);
            }
        );

    unsigned int entityA = registry.addEntity();
    unsigned int entityB = registry.addEntity();

    registry.addComponent<Engine::TagComponent>(entityA, new Engine::TagComponent{"Mario"});
    registry.addComponent<Engine::MaterialComponent>(entityA, new Engine::MaterialComponent{ 0.5f, 0.2f, 0.1f, 1.0f });
    registry.addComponent<Engine::GeometryComponent>(entityA, new Engine::GeometryComponent{ 
        {
            Engine::Math::Vector3{ -1.0, -0.5, 0.0 },
            Engine::Math::Vector3{ 0.0, -0.5, 0.0 },
            Engine::Math::Vector3{ -0.5, 0.5, 0.0 },
            Engine::Math::Vector3{ 1.0, -0.5, 0.0 },
            Engine::Math::Vector3{ 0.5, 0.5, 0.0 }
        },
        {
            0, 1, 2,
            1, 3, 4
        }
    });
    registry.addComponent<Engine::TransformComponent>(entityA, new Engine::TransformComponent{});

    registry.addComponent<Engine::OpenGLRenderComponent>(entityA, new Engine::OpenGLRenderComponent{registry});

    registry.addComponent<Engine::TagComponent>(entityB, new Engine::TagComponent{"Luigi"});

    GLFWwindow* window = Window::getWindow();

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