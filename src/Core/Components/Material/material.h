#ifndef CORE_COMPONENTS_MATERIAL
#define CORE_COMPONENTS_MATERIAL

#include "../../Math/math.h"

namespace Engine
{
class MaterialComponent
{
private:
    Math::Vector4 m_diffuseColor;
    Math::Vector4 m_specularColor;
    float m_specularExponent{100};

public:
    MaterialComponent();
    MaterialComponent(float r, float g, float b, float a);
    MaterialComponent(Math::Vector4 &color);

    Math::Vector4 &getDiffuseColor();
    void setDiffuseColor(const Math::Vector4 &color);

    Math::Vector4 &getSpecularColor();
    void setSpecularColor(const Math::Vector4 &color);

    float getSpecularExponent() const;
    float &getSpecularExponent();
    void setSpecularExponent(float exp);
};
} // namespace Engine

#endif