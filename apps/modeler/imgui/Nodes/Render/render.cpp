#include "render.h"

void UICreation::drawRenderNode(Engine::Registry& registry) {
    std::shared_ptr<Engine::OpenGLRenderComponent> render = registry.getComponent<Engine::OpenGLRenderComponent>(selectedEntity);

    createComponentNodeOutline<Engine::OpenGLRenderComponent>("Render", registry, render.get(), [&]() {
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