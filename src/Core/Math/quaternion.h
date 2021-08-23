#ifndef ENGINE_CORE_MATH_QUATERNION_H
#define ENGINE_CORE_MATH_QUATERNION_H

#include <mathlib/mathlib.h>

namespace Engine
{

class Quaternion
{
protected:
    MathLib::Vector<float, 3> m_qv{};
    float m_qw{};

public:
    Quaternion();

    Quaternion(const MathLib::Vector<float, 3> &imaginary, float real);

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

    MathLib::Vector<float, 3> &qv();

    const MathLib::Vector<float, 3> &qv() const;

    Quaternion getConjugate() const;

    float norm() const;

    Quaternion &setIdentity();

    Quaternion getInverse() const;

    Quaternion &setUnit();

    Quaternion getUnit();

    Quaternion &setRotation(const MathLib::Vector<float, 3> &axis, float angle);

    Quaternion &operator*=(const Quaternion &other);
};

Quaternion operator*(const Quaternion &q, const Quaternion &r);

Quaternion operator*(float scalar, const Quaternion &q);

Quaternion operator*(const Quaternion &q, float scalar);

Quaternion operator+(const Quaternion &q, const Quaternion &r);

Quaternion slerp(const Quaternion &q, const Quaternion &r, float stepSize);

bool operator==(const Quaternion &q1, const Quaternion &q2);

} // namespace Engine

#endif