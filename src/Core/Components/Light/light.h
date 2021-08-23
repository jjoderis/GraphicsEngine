#ifndef CORE_COMPONENTS_LIGHT
#define CORE_COMPONENTS_LIGHT

#include "../../Math/math.h"

namespace Engine
{
class Light
{
public:
    void setColor(const Vector3 &color);
    Vector3 &getColor();

protected:
    Light();
    Light(const Vector3 &color);
    Vector3 m_color{1.0, 1.0, 1.0};
};

class AmbientLightComponent : public Light
{
};

class DirectionalLightComponent : virtual public Light
{
public:
    DirectionalLightComponent();
    DirectionalLightComponent(const Vector3 &color, const Vector3 &direction);

    void setDirection(const Vector3 &direction);
    Vector3 &getDirection();

private:
    Vector3 m_direction{0.0, 0.0, 1.0};
};

class PunctualLight : virtual public Light
{
public:
    void setPosition(const Vector3 &position);
    Vector3 &getPosition();

    void setIntensity(float intensity);
    float getIntensity();

protected:
    PunctualLight();
    PunctualLight(const Vector3 &color, const Vector3 &position, float intensity);
    Vector3 m_position{0.0, 0.0, 0.0};
    float m_intensity{1.0};
};

class PointLightComponent : public PunctualLight
{
public:
    PointLightComponent();
    PointLightComponent(const Vector3 &color, const Vector3 &position, float intensity);
};

class SpotLightComponent : public DirectionalLightComponent, public PunctualLight
{
public:
    SpotLightComponent();
    SpotLightComponent(const Vector3 &color, const Vector3 &position, float intensity, float cutoff, float penumbra);

    void setCutoff(float angle);
    float getCutoff();

    void setPenumbra(float angle);
    float getPenumbra();

private:
    // angle at which the light gets cut off
    float m_cutoffAngle{M_PI_4};
    // angle at which the spot with the most intense light ends
    float m_penumbraAngle{M_PI_4 / 2};
};
} // namespace Engine

#endif