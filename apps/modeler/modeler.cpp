// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "glad/opengl.h"
#include "glfw/window.h"
#include "imgui/imgui.h"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <Core/Components/Camera/camera.h>
#include <Core/Components/Geometry/geometry.h>
#include <Core/Components/Hierarchy/hierarchy.h>
#include <Core/Components/Light/light.h>
#include <Core/Components/Render/render.h>
#include <Core/Components/Tag/tag.h>
#include <Core/Components/Transform/transform.h>
#include <Core/Systems/HierarchyTracker/hierarchyTracker.h>
#include <ECS/registry.h>
#include <Math/math.h>
#include <OpenGL/Components/Material/material.h>
#include <OpenGL/Components/Shader/shader.h>
#include <OpenGL/Components/Texture/texture.h>
#include <OpenGL/Renderer/renderer.h>
#include <OpenGL/Systems/GeometryTracker/geometryTracker.h>
#include <OpenGL/Systems/MaterialTracker/materialTracker.h>
#include <OpenGL/Systems/ShaderTracker/shaderTracker.h>
#include <OpenGL/Systems/TransformTracker/transformTracker.h>
#include <OpenGL/Util/textureIndex.h>
#include <Raytracing/Components/Hittable/hittable.h>
#include <Raytracing/Components/Material/raytracingMaterial.h>
#include <Systems/RenderTracker/renderTracker.h>
#include <cstring>

#include <iostream>

GLFWwindow *Window::m_window = nullptr;
Engine::Registry registry{};
Engine::Util::OpenGLTextureIndex textureIndex{};

void addSphere(Engine::Registry &registry,
               const char *name,
               const Engine::Vector3 &position,
               float radius,
               const Engine::Vector3 &color,
               int material,
               float refractionIndex = 0)
{
    unsigned int sphere{registry.addEntity()};
    registry.createComponent<Engine::TagComponent>(sphere, name);

    auto transform{registry.createComponent<Engine::TransformComponent>(sphere)};
    transform->translate(position);
    transform->update();

    registry.addComponent<Engine::Hittable>(sphere,
                                            std::make_shared<Engine::HittableSphere>(Engine::Point3{0, 0, 0}, radius));

    if (material == 0)
    {
        registry.addComponent<Engine::RaytracingMaterial>(sphere, std::make_shared<Engine::LambertianMaterial>(color));
    }
    else if (material == 1)
    {
        registry.addComponent<Engine::RaytracingMaterial>(sphere, std::make_shared<Engine::MetalMaterial>(color, 0));
    }
    else
    {
        registry.addComponent<Engine::RaytracingMaterial>(
            sphere, std::make_shared<Engine::DielectricMaterial>(refractionIndex));
    }
}

int main()
{
    Window::init();
    OpenGL::init();

    Engine::OpenGLRenderer *renderer = new Engine::OpenGLRenderer{registry};
    Engine::Systems::HierarchyTracker hierarchyTracker{registry};
    Engine::Systems::OpenGLGeometryTracker geometryTracker{registry};
    Engine::Systems::OpenGLTransformTracker transFormTracker{registry};
    Engine::Systems::OpenGLShaderTracker shaderTracker{registry};
    Engine::Systems::OpenGLMaterialTracker materialTracker{registry};

    // we have to delete the renderer before we tear down the OpenGL Context => we have to be able to delete the
    // renderer before this function ends
    GLFWwindow *window = Window::getWindow();

    UI::init(registry, *renderer, textureIndex);

    unsigned int light1{registry.addEntity()};
    registry.createComponent<Engine::TagComponent>(light1, "Light 1");
    auto pointLight{registry.createComponent<Engine::PointLightComponent>(light1)};

    pointLight->setIntensity(0.3);

    registry.createComponent<Engine::TransformComponent>(light1);

    addSphere(registry, "Center Sphere", {0.0f, 0.0f, -1.0f}, 0.5, {0.1, 0.2, 0.5}, 0);

    addSphere(registry, "Left Sphere", {-1.0f, 0.0f, -1.0f}, 0.5, {0.8, 0.8, 0.8}, 2, 1.5);

    addSphere(registry, "Right Sphere", {1.0f, 0.0f, -1.0f}, 0.5, {0.8, 0.6, 0.2}, 1);

    addSphere(registry, "Ground Sphere", {0.0f, -100.5f, -1.0f}, 100, {0.8, 0.8, 0.0}, 0);

    while (!glfwWindowShouldClose(window))
    {
        Window::preRender();

        OpenGL::preRender();

        UI::preRender();

        UI::render(registry);

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