#include "math.h"

Engine::Math::Matrix4 Engine::Math::getTranslation(const Engine::Math::Vector3 &position)
{
    return MathLib::getTranslation(position);
}

Engine::Math::Matrix4 Engine::Math::getScaling(const Vector3 &scaling) {
    return MathLib::getScaling(scaling);
}

Engine::Math::Matrix4 Engine::Math::getRotation(const Engine::Math::Vector3 &rotation) {
    return MathLib::getRotation(rotation);
}

Engine::Math::Vector3 Engine::Math::extractEuler(const Matrix4& rotationMat) {
    float head{atan2(-rotationMat(2, 0), rotationMat(2, 2))};
    float pitch{asin(rotationMat(2, 1))};
    float roll{atan2(-rotationMat(0, 1), rotationMat(1, 1))};

    return Vector3{pitch, head, roll};
}

Engine::Math::Matrix4 Engine::Math::lookAt(const Vector3& position, const Vector3& up, const Vector3& target) {
    Vector3 v = (target - position).normalize();
    Vector3 r = -cross(v, up).normalize();
    Vector3 u = cross(v, r);

    return Matrix4{
        r(0), r(1), r(2), dot(-position, r),
        u(0), u(1), u(2), dot(-position, u),
        v(0), v(1), v(2), dot(-position, v),
           0,    0,    0,                 1,
    };
}