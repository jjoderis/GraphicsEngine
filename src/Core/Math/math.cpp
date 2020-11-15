#include "math.h"

Engine::Math::Matrix4 Engine::Math::getTranslation(Engine::Math::Vector3 &position)
{
    return MathLib::getTranslation(position);
}

Engine::Math::Matrix4 Engine::Math::getScaling(Vector3 &scaling) {
    return MathLib::getScaling(scaling);
}

Engine::Math::Matrix4 Engine::Math::getRotation(Engine::Math::Vector3 &rotation) {
    return MathLib::getRotateX(rotation(0)) * MathLib::getRotateY(rotation(1)) * MathLib::getRotateZ(rotation(2));
}