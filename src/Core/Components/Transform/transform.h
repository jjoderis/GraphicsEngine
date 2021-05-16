#ifndef CORE_COMPONENTS_TRANSFORM
#define CORE_COMPONENTS_TRANSFORM

#include "../../Math/math.h"

namespace Engine
{
class TransformComponent
{
private:
    Math::Vector3 m_translation{0.0f, 0.0f, 0.0f};
    Math::Vector3 m_rotation{0.0f, 0.0f, 0.0f};
    Math::Vector3 m_scaling{1.0f, 1.0f, 1.0f};

    Math::Matrix4 m_modelMatrix{};
    Math::Matrix4 m_normalMatrix{};
    Math::Matrix4 m_modelMatrixInverse{};

public:
    TransformComponent();

    void translate(const Math::Vector3 &translation);
    void setTranslation(const Math::Vector3 &translation);
    Math::Vector3 &getTranslation();

    void scale(const Math::Vector3 &scaling);
    void setScale(const Math::Vector3 &scaling);
    Math::Vector3 &getScaling();

    void rotate(const Math::Vector3 &rotation);
    void setRotation(const Math::Vector3 &rotation);
    Math::Vector3 &getRotation();

    void update();
    Math::Matrix4 &getModelMatrix();
    Math::Matrix4 &getNormalMatrix();
    Math::Matrix4 &getModelMatrixInverse();
};
} // namespace Engine

#endif