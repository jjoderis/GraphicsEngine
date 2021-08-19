#ifndef CORE_COMPONENTS_TRANSFORM
#define CORE_COMPONENTS_TRANSFORM

#include "../../Math/math.h"

namespace Engine
{
class TransformComponent
{
public:
    TransformComponent();

    void translate(const Math::Vector3 &translation);
    void setTranslation(const Math::Vector3 &translation);
    Math::Vector3 &getTranslation();

    void scale(const Math::Vector3 &scaling);
    void setScale(const Math::Vector3 &scaling);
    Math::Vector3 &getScaling();

    // Rotate using Euler Angles
    void rotate(const Math::Vector3 &rotation);
    void setRotation(const Math::Vector3 &rotation);
    Math::Vector3 getEulerRotation();

    // Rotate using Quaternions
    void rotate(Math::Quaternion quat);
    void rotate(float angle, const Math::Vector3 &axis);
    void setRotation(const Math::Quaternion &quat);
    void setRotation(float angle, const Math::Vector3 &axis);
    Math::Quaternion &getRotation();

    void update();
    Math::Matrix4 &getModelMatrix();
    Math::Matrix4 &getModelMatrixInverse();

    Math::Matrix4 &getMatrixWorld();
    Math::Matrix4 &getMatrixWorldInverse();

    Math::Matrix4 &getNormalMatrix();
    Math::Matrix4 &getNormalMatrixInverse();

    Math::Matrix4 &getNormalMatrixWorld();
    Math::Matrix4 &getNormalMatrixWorldInverse();

private:
    Math::Vector3 m_translation{0.0f, 0.0f, 0.0f};
    Math::Vector3 m_scaling{1.0f, 1.0f, 1.0f};
    Math::Quaternion m_rotation{0.0, 0.0, 0.0, 1.0};

    Math::Matrix4 m_modelMatrix{};
    Math::Matrix4 m_modelMatrixInverse{};

    Math::Matrix4 m_matrixWorld{};
    Math::Matrix4 m_matrixWorldInverse{};

    Math::Matrix4 m_normalMatrix{};
    Math::Matrix4 m_normalMatrixInverse{};

    Math::Matrix4 m_normalMatrixWorld{};
    Math::Matrix4 m_normalMatrixWorldInverse{};
};
} // namespace Engine

#endif