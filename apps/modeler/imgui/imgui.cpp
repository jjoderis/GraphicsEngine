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

void drawEntityNode(unsigned int entity, Core::Registry &registry) {
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

void drawMaterialNode(Core::Registry &registry) {
    Engine::MaterialComponent* material = registry.getComponent<Engine::MaterialComponent>(selectedEntity);

    if (ImGui::CollapsingHeader("Material")) {
        ImGui::ColorEdit4("Color", material->getColor().raw());
    }
}

void drawGeometryNode(Core::Registry &registry) {
    Engine::GeometryComponent* geometry = registry.getComponent<Engine::GeometryComponent>(selectedEntity);

    if (ImGui::CollapsingHeader("Geometry")) {
        if (ImGui::TreeNode("Vertices")) { 
            std::vector<Engine::Math::Vector3> &vertices{geometry->getVertices()};
            for(int i = 0; i < vertices.size(); ++i) {
                std::string str = std::to_string(i);
                ImGui::DragFloat3(str.c_str(), vertices[i].raw(), 0.1);
            }

            ImGui::TreePop();
        }
    }
}

void UI::render(Core::Registry &registry) {
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