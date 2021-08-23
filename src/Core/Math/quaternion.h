#ifndef ENGINE_CORE_MATH_QUATERNION_H
#define ENGINE_CORE_MATH_QUATERNION_H

#include "./vector.h"

namespace Engine
{

namespace Math
{

class Quaternion
{
protected:
    TypeVector3<float> m_qv{};
    float m_qw{};

public:
    Quaternion();

    Quaternion(const TypeVector3<float> &imaginary, float real);

    Quaternion(float x, float y, float z, float w);

    Quaternion(const Quaternion &other);

    Quaternion &operator=(const Quaternion &other);

    float &qx();

    float qx() const;

    float &qy();

    float qy() const;

    float &qz();

    float qz() const;

    float &qw();

    float qw() const;

    TypeVector3<float> &qv();

    const TypeVector3<float> &qv() const;

    Quaternion getConjugate() const;

    float norm() const;

    Quaternion &setIdentity();

    Quaternion getInverse() const;

    Quaternion &setUnit();

    Quaternion getUnit();

    Quaternion &setRotation(const TypeVector3<float> &axis, float angle);

    Quaternion &operator*=(const Quaternion &other);
};

Quaternion operator*(const Quaternion &q, const Quaternion &r);

Quaternion operator*(float scalar, const Quaternion &q);

Quaternion operator*(const Quaternion &q, float scalar);

Quaternion operator+(const Quaternion &q, const Quaternion &r);

Quaternion slerp(const Quaternion &q, const Quaternion &r, float stepSize);

bool operator==(const Quaternion &q1, const Quaternion &q2);

} // namespace Math
} // namespace Engine

#endif