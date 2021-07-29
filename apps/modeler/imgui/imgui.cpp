#include "imgui.h"

#include "./Util/SceneLoading/sceneLoader.h"
#include "Nodes/Camera/camera.h"
#include "Nodes/Entity/entity.h"
#include "Nodes/Geometry/geometryNode.h"
#include "Nodes/Light/light.h"
#include "Nodes/OpenGLMaterial/openGLMaterial.h"
#include "Nodes/OpenGLShader/openGLShader.h"
#include "Nodes/Texture/texture.h"
#include "Nodes/Transform/transform.h"
#include "Nodes/helpers.h"
#include "OpenGL/Components/Texture/texture.h"
#include "Util/errorModal.h"
#include "Util/fileBrowser.h"
#include "Window/Raytracing/raytracingWindow.h"
#include <Core/Components/Camera/camera.h>
#include <Core/Components/Geometry/geometry.h>
#include <Core/Components/Hierarchy/hierarchy.h>
#include <Core/Components/Light/light.h>
#include <Core/Components/Tag/tag.h>
#include <Core/Components/Transform/transform.h>
#include <Core/ECS/util.h>
#include <Core/Math/math.h>
#include <Core/Util/Raycaster/raycaster.h>
#include <OpenGL/Components/Shader/shader.h>
#include <Raytracing/Components/Material/raytracingMaterial.h>
#include <Util/fileHandling.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <OpenGL/Util/textureIndex.h>
#include <cstring>
#include <imgui.h>

#include "../glfw/window.h"
#include <GLFW/glfw3.h>

extern Engine::Util::OpenGLTextureIndex textureIndex;

std::vector<UICreation::ComponentWindow*> componentWindows{};

bool showDemoWindow{false};
unsigned int mainCamera{0};
bool dragging{1};

using namespace UICreation;
int selectedEntity = -1;
int possible_component_current = 0;

Engine::Math::Vector3 debugOrigin{0, 0, 0};
Engine::Math::Vector3 debugDirection{0, 0, 0};

void UI::init(Engine::Registry &registry, Engine::Util::OpenGLTextureIndex &textureIndex)
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

    ImGui::StyleColorsDark();

    GLFWwindow *window = Window::getWindow();

    ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR |
                               ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    int width, height;

    glfwGetFramebufferSize(window, &width, &height);

    mainCamera = registry.addEntity();
    registry.createComponent<Engine::TagComponent>(mainCamera, "Modeler Camera");
    auto transform = registry.createComponent<Engine::TransformComponent>(mainCamera);
    transform->setRotation(Engine::Math::Vector3{0.0, M_PI, 0.0});
    transform->update();
    auto camera = registry.createComponent<Engine::CameraComponent>(mainCamera, registry);
    camera->updateAspect((float)width / (float)height);
    camera->getFar() = 300;
    registry.updated<Engine::CameraComponent>(mainCamera);

    UIUtil::initFileBrowserIcons();

    componentWindows.emplace_back(new TransformComponentWindow{selectedEntity, registry});
    componentWindows.emplace_back(new CameraComponentWindow{selectedEntity, registry});
    componentWindows.emplace_back(new GeometryComponentWindow{selectedEntity, registry});
    componentWindows.emplace_back(new LightComponentWindow{selectedEntity, registry});
    componentWindows.emplace_back(new MaterialComponentWindow{selectedEntity, registry});
    componentWindows.emplace_back(new OpenGLShaderComponentWindow{selectedEntity, registry});
    componentWindows.emplace_back(new TextureComponentWindow{selectedEntity, registry, textureIndex});
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
        if (ImGui::Button("Import##Geometry"))
        {
            UIUtil::can_open_function = [](const fs::path &path) -> bool
            {
                // we can import the file if it has an .off extension
                GLenum fileType;

                return (fs::is_regular_file(path) && path.extension().string() == ".off");
            };
            UIUtil::open_function = [&registry](const fs::path &path, const std::string &fileName)
            { registry.addComponent<Engine::GeometryComponent>(selectedEntity, Engine::loadOffFile(path)); };
            UIUtil::openFileBrowser();
        }
        if (ImGui::Button("Blank"))
        {
            registry.createComponent<Engine::GeometryComponent>(selectedEntity);
        }
        if (ImGui::Button("Triangle"))
        {
            auto geometry = std::make_shared<Engine::GeometryComponent>(
                std::initializer_list<Engine::Math::Vector3>{Engine::Math::Vector3{0.5, -0.5, 0.0},
                                                             Engine::Math::Vector3{-0.5, -0.5, 0.0},
                                                             Engine::Math::Vector3{0.0, 0.5, 0.0}},
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

// template <typename ComponentType>
// void drawComponentNode(const char *componentName, Engine::Registry &registry)
// {
//     if (registry.hasComponent<ComponentType>(selectedEntity))
//     {
//         bool isOpen = UICreation::createComponentNodeStart<ComponentType>(componentName);
//         bool wasRemoved = UICreation::createHeaderControls<ComponentType>(componentName, registry);

//         if (isOpen && !wasRemoved)
//         {
//             std::shared_ptr<ComponentType> component = registry.getComponent<ComponentType>(selectedEntity);
//             UICreation::createComponentNodeMain<ComponentType>(component, registry);
//         }

//         UICreation::createComponentNodeEnd();
//     }
// }

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
            UICreation::showRaytracingWindow(registry);
        }

        if (ImGui::Button("Export Scene"))
        {
            UIUtil::can_open_function = [](const fs::path &path) -> bool { return fs::is_directory(path); };
            UIUtil::open_function = [&registry](const fs::path &path, const std::string &fileName)
            { Engine::Util::saveScene(path, registry, textureIndex); };
            UIUtil::openFileBrowser();
        }

        if (ImGui::Button("Import Scene"))
        {
            UIUtil::can_open_function = [](const fs::path &path) -> bool
            { return fs::is_regular_file(path) && path.extension().string() == ".gltf"; };
            UIUtil::open_function = [&registry](const fs::path &path, const std::string &fileName)
            {
                registry.clear();
                Engine::Util::loadScene(path, registry, textureIndex);
            };
            UIUtil::openFileBrowser();
        }

        UICreation::drawEntitiesNode(registry);

        if (selectedEntity > -1)
        {
            const char *possibleComponents[]{"",
                                             "Material",
                                             "Geometry",
                                             "Transform",
                                             "Render",
                                             "Camera",
                                             "Ambient Light",
                                             "Directional Light",
                                             "Point Light",
                                             "Spot Light",
                                             "Bounding Box"};
            if (ImGui::BeginCombo("##Available Components", possibleComponents[possible_component_current]))
            {
                if (!registry.hasComponent<Engine::OpenGLMaterialComponent>(selectedEntity) &&
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
                if (!strcmp(possibleComponents[possible_component_current], "Material"))
                {
                    registry.createComponent<Engine::OpenGLMaterialComponent>(selectedEntity);
                    registry.createComponent<Engine::RaytracingMaterial>(selectedEntity);
                }
                else if (!strcmp(possibleComponents[possible_component_current], "Geometry"))
                {
                    ImGui::OpenPopup("Select Geometry Type");
                }
                else if (!strcmp(possibleComponents[possible_component_current], "Transform"))
                {
                    registry.createComponent<Engine::TransformComponent>(selectedEntity);
                }
                else if (!strcmp(possibleComponents[possible_component_current], "Render"))
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
                    registry.updated<Engine::CameraComponent>(mainCamera);
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

            for (auto componentWindow : componentWindows) {
               componentWindow->render();
            }
        }

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                    1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::End();
    }

    UIUtil::drawFileBrowser();

    UICreation::drawRaytracingWindow();

    // catch clicks on the main window
    if (ImGui::IsMouseClicked(0) && !ImGui::GetIO().WantCaptureMouse)
    {
        auto viewport = ImGui::GetMainViewport();
        auto viewportMin = viewport->Pos;
        auto viewportMax = ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y);
        auto mousePos = ImGui::GetMousePos();

        if (mousePos.x > viewportMin.x && mousePos.y > viewportMin.y && mousePos.x < viewportMax.x &&
            mousePos.y < viewportMax.y)
        {
            Engine::Math::IVector2 pixelPosition{mousePos.x - viewportMin.x, mousePos.y - viewportMin.y};
            Engine::Math::IVector2 viewportSize{viewportMax.x - viewportMin.x, viewportMax.y - viewportMin.y};

            unsigned int activeCameraEntity = registry.getOwners<Engine::ActiveCameraComponent>()[0].front();
            auto camera = registry.getComponent<Engine::CameraComponent>(activeCameraEntity);
            Engine::Util::Ray cameraRay = camera->getCameraRay(pixelPosition, viewportSize);

            auto intersections = Engine::Util::castRay(cameraRay, registry);

            if (intersections.size())
            {
                selectedEntity = intersections.begin()->getEntity();
            }
        }
    }

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