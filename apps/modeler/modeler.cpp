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
#include <Raytracing/Components/Material/raytracingMaterial.h>
#include <Systems/RenderTracker/renderTracker.h>
#include <cstring>

#include <iostream>

GLFWwindow *Window::m_window = nullptr;
Engine::Registry registry{};
Engine::Util::OpenGLTextureIndex textureIndex{};

void cameraAspectCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);

    std::vector<std::shared_ptr<Engine::CameraComponent>> cameras = registry.getComponents<Engine::CameraComponent>();

    for (std::shared_ptr<Engine::CameraComponent> &camera : cameras)
    {
        camera->updateAspect((float)width / (float)height);

        const std::list<unsigned int> owners{registry.getOwners<Engine::CameraComponent>(camera)};
        registry.updated<Engine::CameraComponent>(owners.front());
    }
}

int main()
{
    Window::init();
    OpenGL::init();

    std::vector<unsigned int> renderables{};
    Engine::Systems::OpenGLRenderTracker renderTracker{registry, renderables};

    Engine::OpenGLRenderer *renderer = new Engine::OpenGLRenderer{registry};
    Engine::Systems::HierarchyTracker hierarchyTracker{registry};
    Engine::Systems::OpenGLGeometryTracker geometryTracker{registry};
    Engine::Systems::OpenGLTransformTracker transFormTracker{registry};
    Engine::Systems::OpenGLShaderTracker shaderTracker{registry};
    Engine::Systems::OpenGLMaterialTracker materialTracker{registry};

    // we have to delete the renderer before we tear down the OpenGL Context => we have to be able to delete the
    // renderer before this function ends
    GLFWwindow *window = Window::getWindow();

    glfwSetFramebufferSizeCallback(window, cameraAspectCallback);

    UI::init(registry, textureIndex);

    unsigned int light1{registry.addEntity()};
    registry.createComponent<Engine::TagComponent>(light1, "Light 1");
    registry.createComponent<Engine::PointLightComponent>(light1);

    registry.createComponent<Engine::TransformComponent>(light1);
    auto sphereGeometry = Engine::createSphereGeometry(1.0, 20, 20);
    registry.addComponent<Engine::GeometryComponent>(light1, sphereGeometry);

    unsigned int object1{registry.addEntity()};
    registry.createComponent<Engine::TagComponent>(object1, "Object 1");
    auto material = registry.createComponent<Engine::OpenGLMaterialComponent>(object1);
    // TODO: nicer way to set data
    material->setMaterialData(
        Engine::ShaderMaterialData{48,
                                   std::vector<Engine::MaterialUniformData>{{"diffuseColor", GL_FLOAT_VEC4, 0},
                                                                            {"specularColor", GL_FLOAT_VEC4, 16},
                                                                            {"specularExponent", GL_FLOAT, 32}}});
    float defaultData[48]{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 100.0, 0.0, 0.0, 0.0};
    float *properties{material->getProperty<float>(0)};
    std::memcpy(properties, defaultData, 9 * sizeof(float));
    registry.createComponent<Engine::OpenGLTextureComponent>(object1)->addTexture(
        textureIndex.needTexture("../../data/textures/earth.jpg", GL_TEXTURE_2D));

    registry.createComponent<Engine::RaytracingMaterial>(object1);

    auto transform = registry.createComponent<Engine::TransformComponent>(object1);
    transform->translate(Engine::Math::Vector3{0.0f, 0.0f, 4.0f});
    transform->update();
    auto geometry = registry.addComponent<Engine::GeometryComponent>(object1, sphereGeometry);
    registry.createComponent<Engine::OpenGLShaderComponent>(
        object1, Engine::loadShaders("../../data/shaders/Phong_Sphere_Texture"));
    registry.createComponent<Engine::RenderComponent>(object1);

    unsigned int object2{registry.addEntity()};
    registry.createComponent<Engine::TagComponent>(object2, "Object 2");
    auto hierarchy = registry.createComponent<Engine::HierarchyComponent>(object2);
    hierarchy->setParent(object1);
    registry.updated<Engine::HierarchyComponent>(object2);

    while (!glfwWindowShouldClose(window))
    {
        Window::preRender();

        OpenGL::preRender();

        UI::preRender();

        UI::render(registry);

        renderer->render(renderables);

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