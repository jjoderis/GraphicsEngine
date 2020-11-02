#include "math.h"
#include <math.h>

float Engine::Math::degToRad(float deg)
{
    return (deg * M_PI) / 180;
}

float Engine::Math::radToDeg(float rad)
{
    return (rad * 180) / M_PI;
}

Engine::Math::Matrix4 Engine::Math::getTranslation(Engine::Math::Vector3 &position)
{
    return Matrix4{
        1.0f, 0.0f, 0.0f, position(0),
        0.0f, 1.0f, 0.0f, position(1),
        0.0f, 0.0f, 1.0f, position(2),
        0.0f, 0.0f, 0.0f, 1.0f
    };
}