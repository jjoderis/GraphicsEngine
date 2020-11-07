#ifndef CORE_MATH_MATH_H
#define CORE_MATH_MATH_H

#include <mathlib/mathlib.h>

namespace Engine {
    namespace Math {
        using Vector2 = MathLib::Vector<float, 2>;
        using Vector3 = MathLib::Vector<float, 3>;
        using Vector4 = MathLib::Vector<float, 4>;

        using IVector2 = MathLib::Vector<int, 2>;
        using IVector3 = MathLib::Vector<int, 3>;
        using IVector4 = MathLib::Vector<int, 4>;

        using DVector2 = MathLib::Vector<double, 2>;
        using DVector3 = MathLib::Vector<double, 3>;
        using DVector4 = MathLib::Vector<double, 4>;

        using Matrix2 = MathLib::Matrix<float, 2, 2>;
        using Matrix3 = MathLib::Matrix<float, 3, 3>;
        using Matrix4 = MathLib::Matrix<float, 4, 4>;

        float degToRad(float deg);
        float radToDeg(float rad);

        Matrix4 getTranslation(Vector3 &location);
        // gets a matrix describing a rotation around the x axis
        // Matrix4 getRotationAroundX(float angle);
    }
}

#endif