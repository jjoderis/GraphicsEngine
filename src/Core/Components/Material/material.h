#ifndef CORE_COMPONENTS_MATERIAL
#define CORE_COMPONENTS_MATERIAL

#include "../../Math/math.h"

namespace Engine
{
class MaterialComponent
{
private:
    Math::Vector4 m_color;

public:
    MaterialComponent();
    MaterialComponent(float r, float g, float b, float a);
    MaterialComponent(Math::Vector4 &color);

    Math::Vector4 &getColor();
};
} // namespace Engine

#endif