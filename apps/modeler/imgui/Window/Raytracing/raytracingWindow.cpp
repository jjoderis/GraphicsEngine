#include "raytracingWindow.h"

#include <Core/Components/Camera/camera.h>
#include <Core/ECS/registry.h>
#include <Core/Math/math.h>
#include <Core/Util/Raycaster/raycaster.h>
#include <Raytracing/raytracer.h>
#include <glad/glad.h>
#include <imgui.h>
#include <vector>

bool showWindow{false};
int width{0};
int height{0};

unsigned int texture{0};

void UICreation::showRaytracingWindow(Engine::Registry &registry)
{
    if (texture)
    {
        glDeleteTextures(1, &texture);
        texture = 0;
    }

    auto viewport = ImGui::GetMainViewport();
    auto viewportMin = viewport->Pos;
    auto viewportMax = ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y);
    width = viewportMax.x - viewportMin.x;
    height = viewportMax.y - viewportMin.y;

    std::vector<float> pixelColors = raytraceScene(registry, width, height);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, pixelColors.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    showWindow = true;
}

void UICreation::hideRaytracingWindow() { showWindow = false; }

void UICreation::drawRaytracingWindow()
{
    if (showWindow)
    {
        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->Size);
        if (ImGui::Begin("Raytracing Window", &showWindow))
        {
            ImGui::Image((void *)texture, ImVec2{width, height});
            ImGui::End();
        }
    }
}