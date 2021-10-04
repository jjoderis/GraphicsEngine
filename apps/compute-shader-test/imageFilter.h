#include <cstdlib>
#include <glad/glad.h>

#include <OpenGL/Util/textureLoader.h>
#include <stb_image_write.h>

#include "./helpers/shaderHelper.h"

void filterImage()
{
    // Color Filter using Conversions between RGB and YCrCb from: https://www.fourcc.org/fccyvrgb.php

    auto program{createComputeProgram(
        "#version 430 core\n"
        "layout (local_size_x = 1, local_size_y = 1) in;"
        "layout (rgba32f, binding = 0) uniform image2D input;"
        "layout (rgba32f, binding = 1) uniform image2D outputTexture;"
        "void main(void){"
        "vec3 base = vec3(16, 128, 128);"
        "mat3 conv = mat3(0.257, 0.439, -0.148, 0.504, -0.368, -0.291, 0.098, -0.071, 0.439);\n"
        "//mat3 conv = mat3(65.481,-37.945,112.439, 128.553,-74.494,-94.154, 24.966,    112.439,  -18.285);\n"
        "vec3 color = vec3(imageLoad(input, ivec2(gl_GlobalInvocationID.xy))) * 256;"
        "color = conv * color + base;"
        "color.y = clamp(color.y, 128, 255);"
        "color.z = clamp(color.z, 0, 128);"
        "vec3 newColor = vec3(0,0,0);"
        "newColor.x = 1.164*(color.x - 16) + 2.018*(color.y - 128);"
        "newColor.y = 1.164*(color.x - 16) - 0.813*(color.z - 128) - 0.391*(color.y - 128);"
        "newColor.z = 1.164*(color.x - 16) + 1.596*(color.z - 128);"
        "newColor = newColor / 256;\n"
        "imageStore(outputTexture, ivec2(gl_GlobalInvocationID.xy), vec4(newColor, 1.0));"
        "}")};

    // there needs to be a input.jpg file in the directory where the executable is located
    auto inTextureInfo{Engine::Util::loadTexture("./input.jpg", GL_TEXTURE_2D, GL_RGBA32F)};

    int dispatchX{inTextureInfo.width};
    int dispatchY{inTextureInfo.height};

    auto outTexture{createOutputTexture(dispatchX, dispatchY)};

    glUseProgram(program);
    glBindImageTexture(0, inTextureInfo.buffer, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, outTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glDispatchCompute(dispatchX, dispatchY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    unsigned char pixels[dispatchX * dispatchY * 3];
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    stbi_write_jpg("filtered.jpg", dispatchX, dispatchY, 3, pixels, 100);
}