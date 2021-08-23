#ifndef CORE_MATH_MATH_H
#define CORE_MATH_MATH_H

#include "./quaternion.h"
#include "./vector.h"
#include <mathlib/mathlib.h>

namespace Engine
{
namespace Math
{
using Vector2 = TypeVector2<float>;
using Vector3 = TypeVector3<float>;
using Vector4 = TypeVector4<float>;

using IVector2 = TypeVector2<int>;
using IVector3 = TypeVector3<int>;
using IVector4 = TypeVector4<int>;

using DVector2 = TypeVector2<double>;
using DVector3 = TypeVector3<double>;
using DVector4 = TypeVector4<double>;

using Matrix2 = MathLib::Matrix<float, 2, 2>;
using Matrix3 = MathLib::Matrix<float, 3, 3>;
using Matrix4 = MathLib::Matrix<float, 4, 4>;

Matrix4 getTranslation(const Vector3 &location);
Matrix4 getScaling(const Vector3 &scaling);
Matrix4 getRotation(const Vector3 &rotation);
Matrix4 getRotation(const Quaternion &quat);

Matrix4 lookAt(const Vector3 &position, const Vector3 &up, const Vector3 &target);

Vector3 extractEuler(const Matrix4 &rotationMat);
} // namespace Math
} // namespace Engine

#endif