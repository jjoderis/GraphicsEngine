#include "math.h"

Engine::Math::Matrix4 Engine::Math::getTranslation(const Engine::Math::Vector3 &position)
{
    return MathLib::getTranslation(position);
}

Engine::Math::Matrix4 Engine::Math::getScaling(const Vector3 &scaling) { return MathLib::getScaling(scaling); }

Engine::Math::Matrix4 Engine::Math::getRotation(const Engine::Math::Vector3 &rotation)
{
    return MathLib::getRotation(rotation);
}

Engine::Math::Matrix4 Engine::Math::getRotation(const Quaternion &quat)
{
    float q0 = quat.qw();
    float q1 = quat.qx();
    float q2 = quat.qy();
    float q3 = quat.qz();
    float q02 = pow(q0, 2);
    float q12 = pow(q1, 2);
    float q22 = pow(q2, 2);
    float q32 = pow(q3, 2);
    return Matrix4{2 * (q02 + q12) - 1,
                   2 * (q1 * q2 - q0 * q3),
                   2 * (q1 * q3 + q0 * q2),
                   0,
                   2 * (q1 * q2 + q0 * q3),
                   2 * (q02 + q22) - 1,
                   2 * (q2 * q3 - q0 * q1),
                   0,
                   2 * (q1 * q3 - q0 * q2),
                   2 * (q2 * q3 + q0 * q1),
                   2 * (q02 + q32) - 1,
                   0,
                   0,
                   0,
                   0,
                   1};
}

Engine::Math::Vector3 Engine::Math::extractEuler(const Matrix4 &rotationMat)
{
    float head{atan2(-rotationMat(2, 0), rotationMat(2, 2))};
    float pitch{asin(rotationMat(2, 1))};
    float roll{atan2(-rotationMat(0, 1), rotationMat(1, 1))};

    return Vector3{pitch, head, roll};
}

Engine::Math::Matrix4 Engine::Math::lookAt(const Vector3 &position, const Vector3 &up, const Vector3 &target)
{
    Vector3 v = normalize((target - position));
    Vector3 r = normalize(-cross(v, up));
    Vector3 u = cross(v, r);

    // clang-format off
    return Matrix4{
        r(0), r(1), r(2), dot(-position, r),
        u(0), u(1), u(2), dot(-position, u),
        v(0), v(1), v(2), dot(-position, v),
           0,    0,    0,                 1,
    };
    // clang-format on
}