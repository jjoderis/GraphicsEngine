#include "imgui.h"

#include "./Window/FileBrowser/fileBrowser.h"
#include "Util/errorModal.h"
#include "Window/Camera/camera.h"
#include "Window/Entity/entity.h"
#include "Window/Geometry/geometryNode.h"
#include "Window/Light/light.h"
#include "Window/Main/mainViewPort.h"
#include "Window/OpenGLMaterial/openGLMaterial.h"
#include "Window/Raytracing/raytracingWindow.h"
#include "Window/Transform/transform.h"
#include <Core/Components/Camera/camera.h>
#include <Core/Components/Geometry/geometry.h>
#include <Core/Components/Hierarchy/hierarchy.h>
#include <Core/Components/Light/light.h>
#include <Core/Components/Tag/tag.h>
#include <Core/Components/Transform/transform.h>
#include <Core/ECS/util.h>
#include <Core/Math/math.h>
#include <OpenGL/Components/Shader/shader.h>
#include <OpenGL/Renderer/renderer.h>
#include <OpenGL/Util/textureIndex.h>
#include <OpenGL/Util/textureLoader.h>
#include <Raytracing/Components/Material/raytracingMaterial.h>
#include <Util/fileHandling.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <cstring>
#include <imgui.h>

#include "../glfw/window.h"
#include <GLFW/glfw3.h>

extern Engine::Util::OpenGLTextureIndex textureIndex;

UICreation::EntityWindow *entityWindow;

UICreation::MainViewPort *mainViewport;
UICreation::RaytracingViewport *raytracingViewport;

UICreation::FileBrowser *fileBrowser;

std::vector<UICreation::ComponentWindow *> componentWindows{};

bool showDemoWindow{false};
bool dragging{1};

namespace fs = std::filesystem;
using namespace UICreation;
int selectedEntity = -1;
int possible_component_current = 0;

void UI::init(Engine::Registry &registry,
              Engine::OpenGLRenderer &renderer,
              Engine::Util::OpenGLTextureIndex &textureIndex)
{
    // setup imgui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    // io.ConfigViewportsNoAutoMerge = true;
    // io.ConfigViewportsNoTaskBarIcon = true;

    Engine::Util::invertTextureOnImportOn();

    ImGui::StyleColorsDark();

    GLFWwindow *window = Window::getWindow();

    ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR |
                               ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    entityWindow = new UICreation::EntityWindow{registry};

    mainViewport = new UICreation::MainViewPort{registry, renderer, selectedEntity, textureIndex};
    raytracingViewport = new UICreation::RaytracingViewport{registry};
    fileBrowser = new UICreation::FileBrowser{registry, textureIndex};

    componentWindows.emplace_back(new TransformComponentWindow{selectedEntity, registry});
    componentWindows.emplace_back(new CameraComponentWindow{selectedEntity, registry});
    componentWindows.emplace_back(new GeometryComponentWindow{selectedEntity, registry});
    componentWindows.emplace_back(new LightComponentWindow{selectedEntity, registry});
    componentWindows.emplace_back(new MaterialComponentWindow{selectedEntity, registry, textureIndex});
}

void UI::preRender()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void drawGeometryTypeSelection(Engine::Registry &registry)
{
    if (ImGui::BeginPopup("Select Geometry Type"))
    {
        if (ImGui::Button("Blank"))
        {
            registry.createComponent<Engine::GeometryComponent>(selectedEntity);
        }
        if (ImGui::Button("Triangle"))
        {
            auto geometry = std::make_shared<Engine::GeometryComponent>(
                std::initializer_list<Engine::Point3>{
                    Engine::Point3{0.5, -0.5, 0.0}, Engine::Point3{-0.5, -0.5, 0.0}, Engine::Point3{0.0, 0.5, 0.0}},
                std::initializer_list<unsigned int>{0, 1, 2});
            geometry->calculateNormals();
            registry.addComponent<Engine::GeometryComponent>(selectedEntity, geometry);
        }
        if (ImGui::Button("Sphere"))
        {
            ImGui::OpenPopup("Sphere Parameters");
        }

        static float sphereRadius{1.0f};
        static int horizontalPoints{10};
        static int verticalPoints{10};
        if (ImGui::BeginPopup("Sphere Parameters"))
        {
            ImGui::InputFloat("Radius", &sphereRadius, 0.0f);
            ImGui::SameLine();
            ImGui::InputInt("Horizontal", &horizontalPoints);
            ImGui::SameLine();
            ImGui::InputInt("Vertical", &verticalPoints);
            ImGui::SameLine();
            if (ImGui::Button("+##add_sphere_geometry"))
            {
                registry.addComponent<Engine::GeometryComponent>(
                    selectedEntity, Engine::createSphereGeometry(sphereRadius, horizontalPoints, verticalPoints));
                sphereRadius = 1.0f;
                horizontalPoints = 10;
                verticalPoints = 10;
            }
            ImGui::EndPopup();
        }

        ImGui::EndPopup();
    }
}

std::vector<fs::path> shaderDirectoryPaths;
std::vector<std::string> shaderDirectoryNames;

void addShaders(Engine::Registry &registry, unsigned int entity, std::shared_ptr<Engine::OpenGLShaderComponent> shader)
{
    registry.addComponent<Engine::OpenGLShaderComponent>(entity, shader);

    if (auto hierarchy = registry.getComponent<Engine::HierarchyComponent>(entity))
    {
        for (unsigned int child : hierarchy->getChildren())
        {
            addShaders(registry, child, shader);
        }
    }
}

void drawShaderTypeSelection(Engine::Registry &registry)
{
    if (ImGui::BeginPopup("Select Shader"))
    {
        for (int i{0}; i < shaderDirectoryPaths.size(); ++i)
        {
            if (ImGui::Button(shaderDirectoryNames[i].c_str()))
            {
                try
                {
                    addShaders(registry,
                               selectedEntity,
                               std::make_shared<Engine::OpenGLShaderComponent>(
                                   Engine::loadShaders(shaderDirectoryPaths[i].c_str())));
                }
                catch (Engine::ShaderException &err)
                {
                    errorMessage = err.what();
                }

                ImGui::CloseCurrentPopup();
            }
        }

        if (errorMessage.size())
        {
            ImGui::OpenPopup("Error Info");
            UIUtil::drawErrorModal(errorMessage);
        }

        ImGui::EndPopup();
    }
}

void UI::render(Engine::Registry &registry)
{

    if (showDemoWindow)
    {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    {
        ImGui::Begin("Hello World!");

        ImGui::Checkbox("Show Demo Window", &showDemoWindow);

        if (ImGui::Button("Raytrace"))
        {
            raytracingViewport->newFrame();
        }

        entityWindow->render();

        if (selectedEntity > -1)
        {
            const char *possibleComponents[]{"",
                                             "Raytracing Material",
                                             "Geometry",
                                             "Transform",
                                             "Shader",
                                             "Camera",
                                             "Ambient Light",
                                             "Directional Light",
                                             "Point Light",
                                             "Spot Light",
                                             "Bounding Box"};
            if (ImGui::BeginCombo("##Available Components", possibleComponents[possible_component_current]))
            {
                if (!registry.hasComponent<Engine::RaytracingMaterial>(selectedEntity) &&
                    ImGui::Selectable(possibleComponents[1], possible_component_current == 1))
                {
                    possible_component_current = 1;
                }
                if (!registry.hasComponent<Engine::GeometryComponent>(selectedEntity) &&
                    ImGui::Selectable(possibleComponents[2], possible_component_current == 2))
                {
                    possible_component_current = 2;
                }
                if (!registry.hasComponent<Engine::TransformComponent>(selectedEntity) &&
                    ImGui::Selectable(possibleComponents[3], possible_component_current == 3))
                {
                    possible_component_current = 3;
                }
                if (!registry.hasComponent<Engine::OpenGLShaderComponent>(selectedEntity) &&
                    ImGui::Selectable(possibleComponents[4], possible_component_current == 4))
                {
                    possible_component_current = 4;
                }
                if (!registry.hasComponent<Engine::CameraComponent>(selectedEntity) &&
                    ImGui::Selectable(possibleComponents[5], possible_component_current == 5))
                {
                    possible_component_current = 5;
                }
                if (!registry.hasComponent<Engine::AmbientLightComponent>(selectedEntity) &&
                    ImGui::Selectable(possibleComponents[6], possible_component_current == 6))
                {
                    possible_component_current = 6;
                }
                if (!registry.hasComponent<Engine::DirectionalLightComponent>(selectedEntity) &&
                    ImGui::Selectable(possibleComponents[7], possible_component_current == 7))
                {
                    possible_component_current = 7;
                }
                if (!registry.hasComponent<Engine::PointLightComponent>(selectedEntity) &&
                    ImGui::Selectable(possibleComponents[8], possible_component_current == 8))
                {
                    possible_component_current = 8;
                }
                if (!registry.hasComponent<Engine::SpotLightComponent>(selectedEntity) &&
                    ImGui::Selectable(possibleComponents[9], possible_component_current == 9))
                {
                    possible_component_current = 9;
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            if (ImGui::Button("+") && possible_component_current)
            {
                if (!strcmp(possibleComponents[possible_component_current], "Raytracing Material"))
                {
                    registry.addComponent<Engine::RaytracingMaterial>(
                        selectedEntity, std::make_shared<Engine::LambertianMaterial>(Engine::Vector3{0.5, 0.5, 0.5}));
                }
                else if (!strcmp(possibleComponents[possible_component_current], "Geometry"))
                {
                    ImGui::OpenPopup("Select Geometry Type");
                }
                else if (!strcmp(possibleComponents[possible_component_current], "Transform"))
                {
                    registry.createComponent<Engine::TransformComponent>(selectedEntity);
                }
                else if (!strcmp(possibleComponents[possible_component_current], "Shader"))
                {
                    ImGui::OpenPopup("Select Shader");
                    shaderDirectoryPaths = Util::getDirectories("../../data/shaders");
                    shaderDirectoryNames.resize(shaderDirectoryPaths.size());
                    for (int i{0}; i < shaderDirectoryPaths.size(); ++i)
                    {
                        shaderDirectoryNames[i] = shaderDirectoryPaths[i].stem().string();
                        for (int j{0}; j < shaderDirectoryNames[i].size(); ++j)
                        {
                            if (shaderDirectoryNames[i].at(j) == '_')
                            {
                                shaderDirectoryNames[i].at(j) = ' ';
                            }
                        }
                    }
                }
                else if (!strcmp(possibleComponents[possible_component_current], "Camera"))
                {
                    auto camera = registry.createComponent<Engine::CameraComponent>(selectedEntity, registry);
                    int width, height;
                    GLFWwindow *window = Window::getWindow();
                    glfwGetFramebufferSize(window, &width, &height);
                    camera->updateAspect((float)width / (float)height);
                    registry.updated<Engine::CameraComponent>(selectedEntity);
                }
                else if (!strcmp(possibleComponents[possible_component_current], "Ambient Light"))
                {
                    registry.createComponent<Engine::AmbientLightComponent>(selectedEntity);
                }
                else if (!strcmp(possibleComponents[possible_component_current], "Directional Light"))
                {
                    registry.createComponent<Engine::DirectionalLightComponent>(selectedEntity);
                }
                else if (!strcmp(possibleComponents[possible_component_current], "Point Light"))
                {
                    registry.createComponent<Engine::PointLightComponent>(selectedEntity);
                }
                else if (!strcmp(possibleComponents[possible_component_current], "Spot Light"))
                {
                    registry.createComponent<Engine::SpotLightComponent>(selectedEntity);
                }
                possible_component_current = 0;
            }
            drawGeometryTypeSelection(registry);
            drawShaderTypeSelection(registry);

            for (auto componentWindow : componentWindows)
            {
                componentWindow->render();
            }
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::End();
    }

    raytracingViewport->render();
    mainViewport->render();

    // UIUtil::drawFileBrowser();

    fileBrowser->render();

    ImGui::Render();
}

void UI::postRender()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO &io = ImGui::GetIO();
    (void)io;

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow *backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    // reset the erroModal so it can be rendered again in the next frame
    errorModalRendered = false;
}