#ifndef ENGINE_RAYTRACING_COMPONENTS_MATERIAL
#define ENGINE_RAYTRACING_COMPONENTS_MATERIAL

#include "../../../Core/Math/math.h"

namespace Engine
{

class RaytracingMaterial
{
public:
    RaytracingMaterial();

    void setColor(const Vector4 &color);
    void setColor(float r, float g, float b, float a);
    Vector4 &getColor();

    void makeReflective();
    void makeUnreflective();
    bool isReflective();

private:
    Vector4 m_color{0, 0, 0, 0};
    bool m_reflective{false};
};

} // namespace Engine

#endif