#ifndef CORE_MATH_VECTOR_H
#define CORE_MATH_VECTOR_H

#include <mathlib/mathlib.h>

namespace Engine
{
template <typename T>
class TypeVector3;

template <typename T>
class TypeVector4;

template <typename T>
class TypeVector2 : public MathLib::Vector<T, 2>
{
public:
    TypeVector2() = default;

    TypeVector2(const MathLib::Vector<T, 2> &vec) : MathLib::Vector<T, 2>{vec} {}

    TypeVector2(T x, T y) : MathLib::Vector<T, 2>{x, y} {}

    TypeVector2(const TypeVector3<T> &other) : MathLib::Vector<T, 2>{other} {}
};

template <typename T>
class TypeVector3 : public MathLib::Vector<T, 3>
{
public:
    TypeVector3() = default;

    TypeVector3(const MathLib::Vector<T, 3> &vec) : MathLib::Vector<T, 3>{vec} {}

    TypeVector3(T x, T y, T z) : MathLib::Vector<T, 3>{x, y, z} {}

    TypeVector3(const TypeVector2<T> &other, T val) : MathLib::Vector<T, 3>{other, val} {}

    TypeVector3(const TypeVector4<T> &other) : MathLib::Vector<T, 3>{other} {}
};

template <typename T>
class TypeVector4 : public MathLib::Vector<T, 4>
{
public:
    TypeVector4() = default;

    TypeVector4(const MathLib::Vector<T, 4> &vec) : MathLib::Vector<T, 4>{vec} {}

    TypeVector4(T x, T y, T z, T w) : MathLib::Vector<T, 4>{x, y, z, w} {}

    TypeVector4(const TypeVector3<T> &other, T val) : MathLib::Vector<T, 4>{other, val} {}
};

template <typename T>
TypeVector2<T> operator*(const MathLib::Matrix<T, 2, 2> &mat, const TypeVector2<T> &vec)
{
    return mat * (MathLib::Vector<T, 2>)vec;
}

template <typename T>
TypeVector3<T> operator*(const MathLib::Matrix<T, 3, 3> &mat, const TypeVector3<T> &vec)
{
    return mat * (MathLib::Vector<T, 3>)vec;
}

template <typename T>
TypeVector4<T> operator*(const MathLib::Matrix<T, 4, 4> &mat, const TypeVector4<T> &vec)
{
    return mat * (MathLib::Vector<T, 4>)vec;
}

} // namespace Engine

#endif