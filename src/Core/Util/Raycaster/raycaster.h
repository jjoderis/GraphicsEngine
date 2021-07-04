#ifndef ENGINE_CORE_UTIL_RAYCASTER
#define ENGINE_CORE_UTIL_RAYCASTER

#include "../../Math/math.h"
#include <set>

namespace Engine
{
class Registry;

namespace Util
{

class Ray
{
public:
    Ray() = delete;
    Ray(const Math::Vector3 &origin, const Math::Vector3 &direction);

    Math::Vector3 getOrigin() const;
    Math::Vector3 getDirection() const;

private:
    Math::Vector3 m_origin;
    Math::Vector3 m_direction;
};

Ray operator*(const Math::Matrix4 &matrix, const Ray &ray);

class RayIntersection
{
public:
    RayIntersection() = delete;
    RayIntersection(const Math::Vector3 &intersection,
                    float distance,
                    unsigned int entity,
                    int face,
                    const Math::Vector2 &baryParams);

    const Math::Vector3 &getIntersection() const;
    int getDistance() const;
    unsigned int getEntity() const;
    int getFace() const;
    const Math::Vector2 &getBaryParams() const;

private:
    Math::Vector3 m_intersection;
    float m_distance;
    unsigned int m_entity;
    int m_face;
    Math::Vector2 m_baryParams{0, 0};
};

bool operator<(const RayIntersection &a, const RayIntersection &b);

std::set<RayIntersection> castRay(Ray &ray, Registry &registry);

} // namespace Util

} // namespace Engine

#endif