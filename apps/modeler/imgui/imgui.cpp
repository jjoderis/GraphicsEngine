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

template <typename ComponentType>
void createImGuiComponentDragSource(ComponentType* component) {
    ImGui::Button("Start Drag");
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        char dragDropType[256]{};
        sprintf(dragDropType, "Component_Drag_%u", Engine::type_index<ComponentType>::value());
    
        // Set payload to carry the index of our item (could be anything)
        ImGui::SetDragDropPayload(dragDropType, &component, sizeof(ComponentType*));

        ImGui::Text("Assign Component");
        ImGui::EndDragDropSource();
    }
}

template <typename ComponentType>
void createImGuiComponentDropTarget(unsigned int entity, Engine::Registry& registry) {
    if (ImGui::BeginDragDropTarget())
    {
        char dragDropType[256]{};
        sprintf(dragDropType, "Component_Drag_%u", Engine::type_index<ComponentType>::value());


        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(dragDropType))
        {
            IM_ASSERT(payload->DataSize == sizeof(ComponentType*));
            ComponentType* payload_n = *(ComponentType**)payload->Data;
            registry.addComponent<ComponentType>(entity, payload_n);
        }
        ImGui::EndDragDropTarget();
    }
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

template <typename ComponentType>
void createComponentNodeOutline(const char* componentName, Engine::Registry& registry, ComponentType* component, std::function<void(void)> drawFunc) {
    // TODO: find out why right click is not always doing something, drag and drop seems to be buggy too (maybe somthing with the ids)
    if (ImGui::CollapsingHeader(componentName)) {
        char buff[64]{'\0'};
        sprintf(buff, "%s_remove_popup", componentName);
        if(ImGui::IsItemClicked(1)) {
            ImGui::OpenPopup(buff);
        }
        // dont draw a component after it was removed
        bool removed{false};
        if (ImGui::BeginPopup(buff))
        {
            sprintf(buff, "Remove %s", componentName);
            if (ImGui::Button(buff)) {
                registry.removeComponent<ComponentType>(selectedEntity);
                removed = true;
            }
            ImGui::EndPopup();
        }
        createImGuiComponentDragSource<ComponentType>(component);
        if (!removed) {
            drawFunc();
        }
    }
}

void drawMaterialNode(Engine::Registry &registry) {
    Engine::MaterialComponent* material = registry.getComponent<Engine::MaterialComponent>(selectedEntity);

    createComponentNodeOutline<Engine::MaterialComponent>("Material", registry, material, [&]() {
        ImGui::ColorEdit4("Color", material->getColor().raw());

        if (ImGui::IsItemEdited()) {
            registry.updated<Engine::MaterialComponent>(selectedEntity);
        }
    });
}

void drawGeometryNode(Engine::Registry &registry) {
    Engine::GeometryComponent* geometry = registry.getComponent<Engine::GeometryComponent>(selectedEntity);

    createComponentNodeOutline<Engine::GeometryComponent>("Geometry", registry, geometry, [&]() {
        if (ImGui::TreeNode("Vertices")) { 
            std::vector<Engine::Math::Vector3> &vertices{geometry->getVertices()};
            for(int i = 0; i < vertices.size(); ++i) {
                std::string str = std::to_string(i);
                str.insert(0, "Vertex ");
                ImGui::DragFloat3(str.c_str(), vertices[i].raw(), 0.1);
                if (ImGui::IsItemEdited()) {
                    registry.updated<Engine::GeometryComponent>(selectedEntity);
                }
            }
            static Engine::Math::Vector3 newVertex{0.0, 0.0, 0.0};
            if (ImGui::Button("+##open_add_vertex_popup")) {
                ImGui::OpenPopup("Add Vertex");
            }
            if (ImGui::BeginPopup("Add Vertex"))
            {
                ImGui::InputFloat3("##new_vertex_input", newVertex.raw());
                ImGui::SameLine();
                if (ImGui::Button("+##add_vertex")) {
                    geometry->addVertex(Engine::Math::Vector3{newVertex});
                    registry.updated<Engine::GeometryComponent>(selectedEntity);
                    newVertex = Engine::Math::Vector3{ 0.0f, 0.0f, 0.0f };
                }
                ImGui::EndPopup();
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
                    registry.updated<Engine::GeometryComponent>(selectedEntity);
                }
            }
            static unsigned int newFace[3]{0u, 0u, 0u};
            if (ImGui::Button("+##open_add_face_popup")) {
                ImGui::OpenPopup("Add Face");
            }
            if (ImGui::BeginPopup("Add Face"))
            {
                ImGui::InputScalarN("##new_face_input", ImGuiDataType_U32, newFace, 3);
                ImGui::SameLine();
                if (ImGui::Button("+##add_face")) {
                    geometry->addFace(newFace[0], newFace[1], newFace[2]);
                    registry.updated<Engine::GeometryComponent>(selectedEntity);
                    newFace[0] = 0u;
                    newFace[1] = 0u;
                    newFace[2] = 0u;
                }
                ImGui::EndPopup();
            }

            ImGui::TreePop();
        }
    });
}

void drawTransformNode(Engine::Registry &registry) {
    Engine::TransformComponent* transform = registry.getComponent<Engine::TransformComponent>(selectedEntity);

    createComponentNodeOutline<Engine::TransformComponent>("Transform", registry, transform, [&]() {
        ImGui::DragFloat3("Translation", transform->getTranslation().raw(), 0.1);
        if(ImGui::IsItemEdited()) {
            transform->update();
            registry.updated<Engine::TransformComponent>(selectedEntity);
        }
        ImGui::DragFloat3("Scaling", transform->getScaling().raw(), 0.1);
        if(ImGui::IsItemEdited()) {
            transform->update();
            registry.updated<Engine::TransformComponent>(selectedEntity);
        }
        
        auto rotDeg = MathLib::Util::radToDeg(transform->getRotation());
        ImGui::DragFloat3("Rotation", rotDeg.raw(), 1.0);
        if(ImGui::IsItemEdited()) {
            transform->setRotation(MathLib::Util::degToRad(rotDeg));
            transform->update();
            registry.updated<Engine::TransformComponent>(selectedEntity);
        }
    });
}

void drawRenderNode(Engine::Registry& registry) {
    Engine::OpenGLRenderComponent* render = registry.getComponent<Engine::OpenGLRenderComponent>(selectedEntity);

    createComponentNodeOutline<Engine::OpenGLRenderComponent>("Render", registry, render, [&]() {
        const char* types[2]{"Points\0", "Triangles\0"};
        static int primitive_type_current = 1;
        const char* comboLabel = types[primitive_type_current];
        static int primitive_type = GL_TRIANGLES;
        if (ImGui::RadioButton("Points", primitive_type == GL_POINTS)) {
            primitive_type = GL_POINTS;
            render->updatePrimitiveType(primitive_type);
        } 
        ImGui::SameLine();
        if (ImGui::RadioButton("Triangles", primitive_type == GL_TRIANGLES)) {
            primitive_type = GL_TRIANGLES;
            render->updatePrimitiveType(primitive_type);
        }
    });
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
            std::vector<const char*> possibleComponents{"Material", "Geometry", "Transform", "Render"};

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

            if (possibleComponents.size()) {
                static int possible_component_current = 0;
                ImGui::Combo("##Available Components", &possible_component_current, possibleComponents.data(), possibleComponents.size());
                ImGui::SameLine();
                if (ImGui::Button("+")) {
                    if (!strcmp(possibleComponents[possible_component_current], "Material")) {
                        registry.addComponent<Engine::MaterialComponent>(selectedEntity, new Engine::MaterialComponent{});
                    } else if (!strcmp(possibleComponents[possible_component_current], "Geometry")) {
                        registry.addComponent<Engine::GeometryComponent>(selectedEntity, new Engine::GeometryComponent{
                            {
                                Engine::Math::Vector3{ -0.5, -0.5, 0.0 },
                                Engine::Math::Vector3{  0.5, -0.5, 0.0 },
                                Engine::Math::Vector3{  0.0,  0.5, 0.0 }
                            },
                            {
                                0, 1, 2
                            }
                        });
                    } else if (!strcmp(possibleComponents[possible_component_current], "Transform")) {
                        registry.addComponent<Engine::TransformComponent>(selectedEntity, new Engine::TransformComponent{});
                    } else if (!strcmp(possibleComponents[possible_component_current], "Render")) {
                        registry.addComponent<Engine::OpenGLRenderComponent>(selectedEntity, new Engine::OpenGLRenderComponent{registry});
                    }
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