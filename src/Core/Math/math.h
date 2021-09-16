#ifndef CORE_MATH_MATH_H
#define CORE_MATH_MATH_H

#include "./quaternion.h"
#include <mathlib/mathlib.h>

namespace Engine
{

using Vector2 = MathLib::Vector<float, 2>;
using Vector3 = MathLib::Vector<float, 3>;
using Vector4 = MathLib::Vector<float, 4>;

using IVector2 = MathLib::Vector<int, 2>;
using IVector3 = MathLib::Vector<int, 3>;
using IVector4 = MathLib::Vector<int, 4>;

using DVector2 = MathLib::Vector<double, 2>;
using DVector3 = MathLib::Vector<double, 3>;
using DVector4 = MathLib::Vector<double, 4>;

using Point3 = MathLib::Point<float, 3>;

using Matrix2 = MathLib::Matrix<float, 2, 2>;
using Matrix3 = MathLib::Matrix<float, 3, 3>;
using Matrix4 = MathLib::Matrix<float, 4, 4>;

class Ray
{
public:
    Ray() = delete;
    Ray(const Point3 &origin, const Vector3 &direction);

    const Point3 &getOrigin() const;
    const Vector3 &getDirection() const;

    Point3 at(double t) const;

private:
    Point3 m_origin;
    Vector3 m_direction;
};

Ray operator*(const Matrix4 &matrix, const Ray &ray);

Matrix4 getTranslation(const Vector3 &location);
Matrix4 getScaling(const Vector3 &scaling);
Matrix4 getRotation(const Vector3 &rotation);
Matrix4 getRotation(const Quaternion &quat);

Matrix4 lookAt(const Vector3 &position, const Vector3 &up, const Vector3 &target);

Vector3 extractEuler(const Matrix4 &rotationMat);

Vector3 randomInUnitSphere();
Vector3 randomUnitVector();
Vector3 randomInHemisphere(const Vector3 &normal);

} // namespace Engine

namespace MathLib
{
Vector<float, 3> operator*(const Matrix<float, 4, 4> &mat, const Vector<float, 3> &vec);
Point<float, 3> operator*(const Matrix<float, 4, 4> &mat, const Point<float, 3> &point);
} // namespace MathLib

#endif