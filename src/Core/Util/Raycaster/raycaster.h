#ifndef ENGINE_CORE_UTIL_RAYCASTER
#define ENGINE_CORE_UTIL_RAYCASTER

#include "../../Math/math.h"
#include <set>

namespace Engine
{
class Registry;

namespace Util
{

class RayIntersection
{
public:
    RayIntersection() = delete;
    RayIntersection(
        const Point3 &intersection, float distance, unsigned int entity, int face, const Vector2 &baryParams);

    const Point3 &getIntersection() const;
    float getDistance() const;
    unsigned int getEntity() const;
    int getFace() const;
    const Vector2 &getBaryParams() const;

private:
    Point3 m_intersection;
    float m_distance;
    unsigned int m_entity;
    int m_face;
    Vector2 m_baryParams{0, 0};
};

bool operator<(const RayIntersection &a, const RayIntersection &b);

std::set<RayIntersection> castRay(Ray &ray, Registry &registry);

} // namespace Util

} // namespace Engine

#endif