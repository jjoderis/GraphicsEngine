#ifndef CORE_COMPONENTS_LIGHT
#define CORE_COMPONENTS_LIGHT

#include "../../Math/math.h"

namespace Engine {
    class Light {
    public:

        void setColor(const Math::Vector3& color);
        Math::Vector3& getColor();
    protected:
        Light();
        Math::Vector3 m_color{ 1.0, 1.0, 1.0 };
    };

    class DirectionalLightComponent : virtual public Light {
    public: 
        DirectionalLightComponent();

        void setDirection(const Math::Vector3& direction);
        Math::Vector3& getDirection();
    private:
        Math::Vector3 m_direction{ 0.0, 0.0, 1.0 };
    };

    class PunctualLight : virtual public Light {
    public:
        void setPosition(const Math::Vector3& position);
        Math::Vector3& getPosition();

        void setIntensity(float intensity);
        float getIntensity();
    protected:
        PunctualLight();
        Math::Vector3 m_position{ 0.0, 0.0, 0.0 };
        float m_intensity{1.0};
    };

    class PointLightComponent : public PunctualLight {
    public: 
        PointLightComponent();
    };

    class SpotLightComponent : public DirectionalLightComponent, public PunctualLight {
    public:
        SpotLightComponent();

        void setCutoff(float angle);
        float getCutoff();

        void setPenumbra(float angle);
        float getPenumbra();
    private:
        // angle at which the light gets cut off
        float m_cutoffAngle{ M_PI_4 };
        // angle at which the spot with the most intense light ends
        float m_penumbraAngle{ M_PI_4 / 2 };
    };
}

#endif