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
    Ray(const Point3 &origin, const Vector3 &direction);

    const Point3 &getOrigin() const;
    const Vector3 &getDirection() const;

private:
    Point3 m_origin;
    Vector3 m_direction;
};

Ray operator*(const Matrix4 &matrix, const Ray &ray);

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