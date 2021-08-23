#ifndef CORE_COMPONENTS_TRANSFORM
#define CORE_COMPONENTS_TRANSFORM

#include "../../Math/math.h"

namespace Engine
{
class TransformComponent
{
public:
    TransformComponent();

    void translate(const Vector3 &translation);
    void setTranslation(const Vector3 &translation);
    Vector3 &getTranslation();

    void scale(const Vector3 &scaling);
    void setScale(const Vector3 &scaling);
    Vector3 &getScaling();

    // Rotate using Euler Angles
    void rotate(const Vector3 &rotation);
    void setRotation(const Vector3 &rotation);
    Vector3 getEulerRotation();

    // Rotate using Quaternions
    void rotate(Quaternion quat);
    void rotate(float angle, const Vector3 &axis);
    void setRotation(const Quaternion &quat);
    void setRotation(float angle, const Vector3 &axis);
    Quaternion &getRotation();

    void update();
    Matrix4 &getModelMatrix();
    Matrix4 &getModelMatrixInverse();

    Matrix4 &getMatrixWorld();
    Matrix4 &getMatrixWorldInverse();

    Matrix4 &getNormalMatrix();
    Matrix4 &getNormalMatrixInverse();

    Matrix4 &getNormalMatrixWorld();
    Matrix4 &getNormalMatrixWorldInverse();

    Matrix4 &getViewMatrix();
    Matrix4 &getViewMatrixInverse();

    Matrix4 &getViewMatrixWorld();
    Matrix4 &getViewMatrixWorldInverse();

private:
    Vector3 m_translation{0.0f, 0.0f, 0.0f};
    Vector3 m_scaling{1.0f, 1.0f, 1.0f};
    Quaternion m_rotation{0.0, 0.0, 0.0, 1.0};

    Matrix4 m_modelMatrix{};
    Matrix4 m_modelMatrixInverse{};

    Matrix4 m_matrixWorld{};
    Matrix4 m_matrixWorldInverse{};

    Matrix4 m_normalMatrix{};
    Matrix4 m_normalMatrixInverse{};

    Matrix4 m_normalMatrixWorld{};
    Matrix4 m_normalMatrixWorldInverse{};

    Matrix4 m_viewMatrix{};
    Matrix4 m_viewMatrixInverse{};

    Matrix4 m_viewMatrixWorld{};
    Matrix4 m_viewMatrixWorldInverse{};
};
} // namespace Engine

#endif