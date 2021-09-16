#ifndef ENGINE_RAYTRACING_HITRECORD_H
#define ENGINE_RAYTRACING_HITRECORD_H

#include "../../Core/Math/math.h"
#include <memory>

// Code from Ray Tracing in One Weekend: https://raytracing.github.io/books/RayTracingInOneWeekend.html

namespace Engine
{
class RaytracingMaterial;

struct HitRecord
{
    Engine::Point3 p;
    Engine::Vector3 normal;
    std::shared_ptr<RaytracingMaterial> matPtr;
    double t;
    bool frontFace;

    void setFaceNormal(const Engine::Ray &r, const Engine::Vector3 &outwardNormal);
};
} // namespace Engine

#endif