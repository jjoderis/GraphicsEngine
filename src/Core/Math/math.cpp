#include "math.h"
#include <math.h>

Engine::Math::Matrix4 Engine::Math::getTranslation(Engine::Math::Vector3 &position)
{
    return Matrix4{
        1.0f, 0.0f, 0.0f, position(0),
        0.0f, 1.0f, 0.0f, position(1),
        0.0f, 0.0f, 1.0f, position(2),
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

Engine::Math::Matrix4 Engine::Math::getScaling(Vector3 &scaling) {
    return Matrix4{
        scaling(0), 0.0f,       0.0f,       0.0f,
        0.0f,       scaling(1), 0.0f,       0.0f,
        0.0f,       0.0f,       scaling(2), 0.0f,
        0.0f,       0.0f,       0.0f,       1.0f
    };
}

Engine::Math::Matrix4 Engine::Math::getRotateX(float radX) {
    return Matrix4 {
        1.0f,      0.0f,       0.0f,      0.0f,
        0.0f, cos(radX), -sin(radX),      0.0f,
        0.0f, sin(radX),  cos(radX),      0.0f,
        0.0f,      0.0f,       0.0f,      1.0f
    };
}

Engine::Math::Matrix4 Engine::Math::getRotateY(float radY) {
    return Matrix4 {
         cos(radY), 0.0f, sin(radY), 0.0f,
         0.0f,      1.0f,      0.0f, 0.0f,
        -sin(radY), 0.0f, cos(radY), 0.0f,
         0.0f,      0.0f,      0.0f, 1.0f
    };
}

Engine::Math::Matrix4 Engine::Math::getRotateZ(float radZ) {
    return Matrix4 {
        cos(radZ), -sin(radZ), 0.0f, 0.0f,
        sin(radZ),  cos(radZ), 0.0f, 0.0f,
             0.0f,       0.0f, 1.0f, 0.0f,
             0.0f,       0.0f, 0.0f, 1.0f
    };
}

Engine::Math::Matrix4 Engine::Math::getRotation(Engine::Math::Vector3 &rotation) {
    return getRotateX(rotation(0)) * getRotateY(rotation(1)) * getRotateZ(rotation(2));
}