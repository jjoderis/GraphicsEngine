#include "raytracingWindow.h"

#include <Core/Components/Camera/camera.h>
#include <Core/ECS/registry.h>
#include <Core/Math/math.h>
#include <Core/Util/Raycaster/raycaster.h>
#include <Raytracing/raytracer.h>
#include <glad/glad.h>
#include <imgui.h>
#include <vector>

UICreation::RaytracingViewport::RaytracingViewport(Engine::Registry &registry)
    : ImGuiWindow("Raytracing Viewport") ,m_registry{registry}
{
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_size.at(0), m_size.at(1), 0, GL_RGB, GL_FLOAT, NULL);
    glGenerateMipmap(GL_TEXTURE_2D); 
}

void UICreation::RaytracingViewport::main() {
    ImGui::Image((void *)m_texture, ImVec2{m_size.at(0), m_size.at(1)});
}

void UICreation::RaytracingViewport::newFrame() {
    std::vector<float> pixelColors = Engine::raytraceScene(m_registry, m_size.at(0), m_size.at(1));

    if (m_texture)
    {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_size.at(0), m_size.at(1), 0, GL_RGB, GL_FLOAT, pixelColors.data());
    glGenerateMipmap(GL_TEXTURE_2D);
}