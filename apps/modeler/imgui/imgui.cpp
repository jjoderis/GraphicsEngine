#include "imgui.h"

bool showDemoWindow{false};
unsigned int mainCamera{0};

using namespace UICreation;
int selectedEntity = -1;
int possible_component_current = 0;

void UI::init(Engine::Registry& registry) {
    //setup imgui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    ImGui::StyleColorsDark();

    GLFWwindow* window = Window::getWindow();

    ImGui::SetColorEditOptions(ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_AlphaBar);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    int width, height;

    glfwGetFramebufferSize(window, &width, &height);

    mainCamera = registry.addEntity();
    registry.addComponent<Engine::TagComponent>(mainCamera, std::make_shared<Engine::TagComponent>("Modeler Camera"));
    std::shared_ptr<Engine::TransformComponent> transform = registry.addComponent<Engine::TransformComponent>(mainCamera, std::make_shared<Engine::TransformComponent>());
    transform->setRotation(Engine::Math::Vector3{0.0, M_PI,0.0});
    transform->update();
    std::shared_ptr<Engine::CameraComponent> camera = registry.addComponent<Engine::CameraComponent>(mainCamera, std::make_shared<Engine::CameraComponent>(registry));
    camera->updateAspect((float)width/(float)height);
    registry.updated<Engine::CameraComponent>(mainCamera);
}

void UI::preRender() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void drawGeometryTypeSelection(Engine::Registry& registry) {
    if (ImGui::BeginPopup("Select Geometry Type"))
    {
        if(ImGui::Button("Blank")) {
            registry.addComponent<Engine::GeometryComponent>(selectedEntity, std::make_shared<Engine::GeometryComponent>());
        }
        if (ImGui::Button("Triangle")) {
            std::shared_ptr<Engine::GeometryComponent> geometry = std::make_shared<Engine::GeometryComponent>(
                std::initializer_list<Engine::Math::Vector3>{
                    Engine::Math::Vector3{  0.5, -0.5, 0.0 },
                    Engine::Math::Vector3{ -0.5, -0.5, 0.0 },
                    Engine::Math::Vector3{  0.0,  0.5, 0.0 }
                },
                std::initializer_list<unsigned int>{
                    0, 1, 2
                }
            );
            geometry->calculateNormals();
            registry.addComponent<Engine::GeometryComponent>(selectedEntity, geometry);
        }
        if (ImGui::Button("Sphere")) {
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
            if (ImGui::Button("+##add_sphere_geometry")) {
                registry.addComponent<Engine::GeometryComponent>(selectedEntity, Engine::createSphereGeometry(sphereRadius, horizontalPoints, verticalPoints));
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

void drawShaderTypeSelection(Engine::Registry& registry) {
    if (ImGui::BeginPopup("Select Shader")) {
        for (int i{0}; i < shaderDirectoryPaths.size(); ++i) {
            if (ImGui::Button(shaderDirectoryNames[i].c_str())) {
                try {
                    registry.addComponent<Engine::OpenGLRenderComponent>(selectedEntity, std::make_shared<Engine::OpenGLRenderComponent>(
                        registry,
                        Engine::loadShaders(shaderDirectoryPaths[i].c_str())
                    ));
                } catch (Engine::ShaderException& err) {
                    errorMessage = err.what();
                }

                ImGui::CloseCurrentPopup();
            }
        }

        if (errorMessage.size()) {
            ImGui::OpenPopup("Error Info"); 
            UIUtil::drawErrorModal(errorMessage);
        }

        ImGui::EndPopup();
    }
}

void UI::render(Engine::Registry &registry) {
    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    {
        ImGui::Begin("Hello World!");

        ImGui::Checkbox("Show Demo Window", &showDemoWindow);

        UICreation::drawEntitiesNode(registry);

        if (selectedEntity > -1) {
            const char* possibleComponents[]{"", "Material", "Geometry", "Transform", "Render", "Camera"};
            if (ImGui::BeginCombo("##Available Components", possibleComponents[possible_component_current]))
            {
                if (!registry.hasComponent<Engine::MaterialComponent>(selectedEntity) && ImGui::Selectable(possibleComponents[1], possible_component_current == 1)) {
                    possible_component_current = 1;
                }
                if (!registry.hasComponent<Engine::GeometryComponent>(selectedEntity) && ImGui::Selectable(possibleComponents[2], possible_component_current == 2)) {
                    possible_component_current = 2;
                }
                if (!registry.hasComponent<Engine::TransformComponent>(selectedEntity) && ImGui::Selectable(possibleComponents[3], possible_component_current == 3)) {
                    possible_component_current = 3;
                }
                if (!registry.hasComponent<Engine::OpenGLRenderComponent>(selectedEntity) && ImGui::Selectable(possibleComponents[4], possible_component_current == 4)) {
                    possible_component_current = 4;
                }
                if (!registry.hasComponent<Engine::CameraComponent>(selectedEntity) && ImGui::Selectable(possibleComponents[5], possible_component_current == 5)) {
                    possible_component_current = 5;
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();
            if (ImGui::Button("+") && possible_component_current) {
                if (!strcmp(possibleComponents[possible_component_current], "Material")) {
                    registry.addComponent<Engine::MaterialComponent>(selectedEntity, std::make_shared<Engine::MaterialComponent>());
                } else if (!strcmp(possibleComponents[possible_component_current], "Geometry")) {
                    ImGui::OpenPopup("Select Geometry Type");
                } else if (!strcmp(possibleComponents[possible_component_current], "Transform")) {
                    registry.addComponent<Engine::TransformComponent>(selectedEntity, std::make_shared<Engine::TransformComponent>());
                } else if (!strcmp(possibleComponents[possible_component_current], "Render")) {
                    ImGui::OpenPopup("Select Shader");
                    shaderDirectoryPaths = Util::getDirectories("../../data/shaders");
                    shaderDirectoryNames.resize(shaderDirectoryPaths.size());
                    for (int i{0}; i < shaderDirectoryPaths.size(); ++i) {
                        shaderDirectoryNames[i] = shaderDirectoryPaths[i].stem().string();
                        for(int j{0}; j < shaderDirectoryNames[i].size(); ++j) {
                            if (shaderDirectoryNames[i].at(j) == '_') {
                                shaderDirectoryNames[i].at(j) = ' ';
                            }
                        }
                    }
                } else if (!strcmp(possibleComponents[possible_component_current], "Camera")) {
                    std::shared_ptr<Engine::CameraComponent> camera = registry.addComponent<Engine::CameraComponent>(selectedEntity, std::make_shared<Engine::CameraComponent>(registry));
                    int width, height;
                    GLFWwindow* window = Window::getWindow();
                    glfwGetFramebufferSize(window, &width, &height);
                    camera->updateAspect((float)width/(float)height);
                    registry.updated<Engine::CameraComponent>(mainCamera);
                }
                possible_component_current = 0;
            }
            drawGeometryTypeSelection(registry);
            drawShaderTypeSelection(registry);

            drawMaterialNode(registry);
            drawGeometryNode(registry);
            drawTransformNode(registry);
            drawRenderNode(registry);
            drawCameraNode(registry);
        }


        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::Render();
}

void UI::postRender() {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    // reset the erroModal so it can be rendered again in the next frame
    errorModalRendered = false;
}