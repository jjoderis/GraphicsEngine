#include "imgui.h"

bool showDemoWindow{false};
unsigned int mainCamera{0};

using namespace UICreation;
int selectedEntity = -1;

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
    registry.addComponent<Engine::TagComponent>(mainCamera, new Engine::TagComponent{"Modeler Camera"});
    Engine::TransformComponent* transform = registry.addComponent<Engine::TransformComponent>(mainCamera, new Engine::TransformComponent{});
    transform->setRotation(Engine::Math::Vector3{0.0, M_PI,0.0});
    transform->update();
    Engine::CameraComponent* camera = registry.addComponent<Engine::CameraComponent>(mainCamera, new Engine::CameraComponent{registry});
    camera->updateAspect((float)width/(float)height);
    registry.updated<Engine::CameraComponent>(mainCamera);
}

void UI::preRender() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}



bool drawEntityNode(unsigned int entity, Engine::Registry &registry) {
    bool removed{ false };
    const std::string& name = registry.getComponent<Engine::TagComponent>(entity)->get();
    bool isOpen = ImGui::TreeNode(name.c_str());
    if (ImGui::IsItemClicked(2)) {
        selectedEntity = entity;
    }
    createImGuiComponentDropTarget<Engine::MaterialComponent>(entity, registry);
    createImGuiComponentDropTarget<Engine::GeometryComponent>(entity, registry);
    createImGuiComponentDropTarget<Engine::TransformComponent>(entity, registry);
    createImGuiComponentDropTarget<Engine::OpenGLRenderComponent>(entity, registry);

    ImGui::SameLine();
    std::string id{"x##"};
    id.append(std::to_string(entity));
    if (ImGui::Button(id.c_str())) {
        removed = true;
    }
    
    if (isOpen)
    {
        ImGui::TreePop();
    }

    return removed;  
}

char name[64];

void UI::render(Engine::Registry &registry) {
    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

    std::list<unsigned int> removals{};

    {
        ImGui::Begin("Hello World!");

        ImGui::Checkbox("Show Demo Window", &showDemoWindow);

        const std::list<unsigned int> &entities{ registry.getEntities() };

        if (selectedEntity > -1) {
            const char* name = registry.getComponent<Engine::TagComponent>(selectedEntity)->get().c_str();
            ImGui::Text("Selection: %s", name);
        } else {
            ImGui::Text("Selection: None selected!");
        }
        
        if (ImGui::CollapsingHeader("Entities"))
        { 
            for (unsigned int entity: entities) {
                if (drawEntityNode(entity, registry)) {
                    removals.push_back(entity);
                }
            } 
            if (ImGui::Button("Add Entity")) {
                ImGui::OpenPopup("entity_add_popup");
            }
            if (ImGui::BeginPopup("entity_add_popup"))
            {
                ImGui::InputTextWithHint("##name_input", "Enter a name", name, IM_ARRAYSIZE(name));
                ImGui::SameLine();
                if(ImGui::Button("+")) {
                    unsigned int newEntity = registry.addEntity();
                    if (!std::strlen(name)) {
                        registry.addComponent<Engine::TagComponent>(newEntity, new Engine::TagComponent{"Unnamed Entity"});
                    } else {
                        registry.addComponent<Engine::TagComponent>(newEntity, new Engine::TagComponent{name});
                    }
                    name[0] = '\0';
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if(ImGui::Button("x")) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            } 
        }

        if (selectedEntity > -1) {
            std::vector<const char*> possibleComponents{"Material", "Geometry", "Transform", "Render", "Camera"};

            if (registry.hasComponent<Engine::MaterialComponent>(selectedEntity)) {
                possibleComponents.erase(std::remove_if(possibleComponents.begin(), possibleComponents.end(), [](const char* compName) {
                    return !strcmp(compName, "Material");
                }), possibleComponents.end());

                drawMaterialNode(registry);
            }
            if (registry.hasComponent<Engine::GeometryComponent>(selectedEntity)) {
                possibleComponents.erase(std::remove_if(possibleComponents.begin(), possibleComponents.end(), [](const char* compName) {
                    return !strcmp(compName, "Geometry");
                }), possibleComponents.end());

                drawGeometryNode(registry);
            }
            if (registry.hasComponent<Engine::TransformComponent>(selectedEntity)) {
                possibleComponents.erase(std::remove_if(possibleComponents.begin(), possibleComponents.end(), [](const char* compName) {
                    return !strcmp(compName, "Transform");
                }), possibleComponents.end());

                drawTransformNode(registry);
            }
            if (registry.hasComponent<Engine::OpenGLRenderComponent>(selectedEntity)) {
                possibleComponents.erase(std::remove_if(possibleComponents.begin(), possibleComponents.end(), [](const char* compName) {
                    return !strcmp(compName, "Render");
                }), possibleComponents.end());

                drawRenderNode(registry);
            }
            if (registry.hasComponent<Engine::CameraComponent>(selectedEntity)) {
                possibleComponents.erase(std::remove_if(possibleComponents.begin(), possibleComponents.end(), [](const char* compName) {
                    return !strcmp(compName, "Camera");
                }), possibleComponents.end());

                drawCameraNode(registry);
            }

            if (possibleComponents.size()) {
                static int possible_component_current = 0;
                ImGui::Combo("##Available Components", &possible_component_current, possibleComponents.data(), possibleComponents.size());
                ImGui::SameLine();
                if (ImGui::Button("+")) {
                    if (!strcmp(possibleComponents[possible_component_current], "Material")) {
                        registry.addComponent<Engine::MaterialComponent>(selectedEntity, new Engine::MaterialComponent{});
                    } else if (!strcmp(possibleComponents[possible_component_current], "Geometry")) {
                        ImGui::OpenPopup("Select Geometry Type");
                    } else if (!strcmp(possibleComponents[possible_component_current], "Transform")) {
                        registry.addComponent<Engine::TransformComponent>(selectedEntity, new Engine::TransformComponent{});
                    } else if (!strcmp(possibleComponents[possible_component_current], "Render")) {
                        registry.addComponent<Engine::OpenGLRenderComponent>(selectedEntity, new Engine::OpenGLRenderComponent{
                            registry,
                            {
                                Engine::OpenGLShader{GL_VERTEX_SHADER, Util::readTextFile("../../data/shaders/Basic_Shading_Shader/basic_shading.vert").c_str()},
                                Engine::OpenGLShader{GL_FRAGMENT_SHADER, Util::readTextFile("../../data/shaders/Basic_Shading_Shader/basic_shading.frag").c_str()},
                            }
                        });
                    } else if (!strcmp(possibleComponents[possible_component_current], "Camera")) {
                        Engine::CameraComponent* camera = registry.addComponent<Engine::CameraComponent>(selectedEntity, new Engine::CameraComponent{registry});
                        int width, height;
                        GLFWwindow* window = Window::getWindow();
                        glfwGetFramebufferSize(window, &width, &height);
                        camera->updateAspect((float)width/(float)height);
                        registry.updated<Engine::CameraComponent>(mainCamera);
                    }
                }
                if (ImGui::BeginPopup("Select Geometry Type"))
                {
                    if(ImGui::Button("Blank")) {
                        registry.addComponent<Engine::GeometryComponent>(selectedEntity, new Engine::GeometryComponent{});
                    }
                    if (ImGui::Button("Triangle")) {
                        Engine::GeometryComponent* geometry = new Engine::GeometryComponent{
                            {
                                Engine::Math::Vector3{  0.5, -0.5, 0.0 },
                                Engine::Math::Vector3{ -0.5, -0.5, 0.0 },
                                Engine::Math::Vector3{  0.0,  0.5, 0.0 }
                            },
                            {
                                0, 1, 2
                            }
                        };
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
        }


        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    for (unsigned int entity: removals) {
        registry.removeEntity(entity);
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
}