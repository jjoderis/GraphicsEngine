#include "imgui.h"

bool showDemoWindow{false};
int selectedEntity{-1};

void UI::init() {
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
}

void UI::preRender() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
}

void drawEntityNode(unsigned int entity, Engine::Registry &registry) {
    const char* name = registry.getComponent<Engine::TagComponent>(entity)->get().c_str();
    bool isOpen = ImGui::TreeNode(name);

    if (ImGui::IsItemClicked(2)) {
        selectedEntity = entity;
    }
    
    if (isOpen)
    {
        ImGui::Text("Itsa me Mario");
        ImGui::TreePop();
    }
    
}

void drawMaterialNode(Engine::Registry &registry) {
    Engine::MaterialComponent* material = registry.getComponent<Engine::MaterialComponent>(selectedEntity);

    if (ImGui::CollapsingHeader("Material")) {
        ImGui::ColorEdit4("Color", material->getColor().raw());

        if (ImGui::IsItemEdited()) {
            if (Engine::OpenGLRenderComponent* renderComponent = registry.getComponent<Engine::OpenGLRenderComponent>(selectedEntity)) {
                renderComponent->updateColor(selectedEntity, material->getColor());
            }
        }
    }
}

void drawGeometryNode(Engine::Registry &registry) {
    Engine::GeometryComponent* geometry = registry.getComponent<Engine::GeometryComponent>(selectedEntity);

    if (ImGui::CollapsingHeader("Geometry")) {
        if (ImGui::TreeNode("Vertices")) { 
            std::vector<Engine::Math::Vector3> &vertices{geometry->getVertices()};
            for(int i = 0; i < vertices.size(); ++i) {
                std::string str = std::to_string(i);
                str.insert(0, "Vertex ");
                ImGui::DragFloat3(str.c_str(), vertices[i].raw(), 0.1);
                // TODO: use isItemEdited as way to update potential RenderComponent when values here are changed 
                if (ImGui::IsItemEdited()) {
                    if (Engine::OpenGLRenderComponent* renderComponent = registry.getComponent<Engine::OpenGLRenderComponent>(selectedEntity)) {
                        renderComponent->updateVertex(selectedEntity, i, vertices[i]);
                    }
                }
            }

            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Faces")) {
            std::vector<unsigned int>& faces{ geometry->getFaces() };
            for(int i = 0; i < faces.size(); i += 3) {
                std::string str = std::to_string(i / 3);
                str.insert(0, "Face ");
                ImGui::InputScalarN(str.c_str(), ImGuiDataType_U32, faces.data() + i, 3);
                if (ImGui::IsItemEdited()) {
                    if (Engine::OpenGLRenderComponent* renderComponent = registry.getComponent<Engine::OpenGLRenderComponent>(selectedEntity)) {
                        renderComponent->updateFace(selectedEntity, i / 3, faces.data() + i);
                    }
                }
            }
            ImGui::TreePop();
        }
    }
}

void drawTransformNode(Engine::Registry &registry) {
    Engine::TransformComponent* transform = registry.getComponent<Engine::TransformComponent>(selectedEntity);

    if (ImGui::CollapsingHeader("Transform")) {
        ImGui::DragFloat3("Translation", transform->getTranslation().raw(), 0.1);
        if(ImGui::IsItemEdited()) {
            transform->update();
            if (Engine::OpenGLRenderComponent* renderComponent = registry.getComponent<Engine::OpenGLRenderComponent>(selectedEntity)) {
                renderComponent->updateTransform(selectedEntity, transform->getModelMatrix());
            }
        }
        ImGui::DragFloat3("Scaling", transform->getScaling().raw(), 0.1);
        if(ImGui::IsItemEdited()) {
            transform->update();
            if (Engine::OpenGLRenderComponent* renderComponent = registry.getComponent<Engine::OpenGLRenderComponent>(selectedEntity)) {
                renderComponent->updateTransform(selectedEntity, transform->getModelMatrix());
            }
        }
        
        auto rotDeg = Engine::Math::radToDeg(transform->getRotation());
        ImGui::DragFloat3("Rotation", rotDeg.raw(), 1.0);
        if(ImGui::IsItemEdited()) {
            transform->setRotation(Engine::Math::degToRad(rotDeg));
            transform->update();
            if (Engine::OpenGLRenderComponent* renderComponent = registry.getComponent<Engine::OpenGLRenderComponent>(selectedEntity)) {
                renderComponent->updateTransform(selectedEntity, transform->getModelMatrix());
            }
        }
    }
}

void UI::render(Engine::Registry &registry) {
    if (showDemoWindow) {
        ImGui::ShowDemoWindow(&showDemoWindow);
    }

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
                drawEntityNode(entity, registry);
            }   
        }

        if (selectedEntity > -1) {
            if (registry.hasComponent<Engine::MaterialComponent>(selectedEntity)) {
                drawMaterialNode(registry);
            }
            if (registry.hasComponent<Engine::GeometryComponent>(selectedEntity)) {
                drawGeometryNode(registry);
            }
            if (registry.hasComponent<Engine::TransformComponent>(selectedEntity)) {
                drawTransformNode(registry);
            }
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
}