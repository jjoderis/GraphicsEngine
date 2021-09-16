#ifndef ENGINE_RAYTRACING_HITTABLE_H
#define ENGINE_RAYTRACING_HITTABLE_H

#include "../../../Core/Math/math.h"
#include <memory>

// Code from Ray Tracing in One Weekend, slightly adapted to fit into our ECS System:
// https://raytracing.github.io/books/RayTracingInOneWeekend.html

namespace Engine
{
class Registry;
class TransformComponent;
class Ray;
class RaytracingMaterial;
class HitRecord;

class Hittable
{
public:
    Hittable(const char *type);

    const char *getType() const;

    virtual bool hit(const Engine::Ray &r,
                     Engine::Registry &registry,
                     unsigned int entity,
                     double t_min,
                     double t_max,
                     HitRecord &rec) const = 0;

private:
    const char *m_type;
};

class HittableSphere : public Hittable
{
public:
    HittableSphere();
    HittableSphere(const Engine::Point3 &cen, double r);

    virtual bool hit(const Engine::Ray &r,
                     Engine::Registry &registry,
                     unsigned int entity,
                     double t_min,
                     double t_max,
                     HitRecord &rec) const override;

private:
    Engine::Point3 m_center{0, 0, 0};
    double m_radius{1};
};

} // namespace Engine

#endif