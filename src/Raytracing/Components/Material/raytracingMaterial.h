#ifndef ENGINE_RAYTRACING_COMPONENTS_MATERIAL
#define ENGINE_RAYTRACING_COMPONENTS_MATERIAL

#include "../../../Core/Math/math.h"
#include <string>

// Code from Ray Tracing in One Weekend, slightly adapted to fit into our ECS System:
// https://raytracing.github.io/books/RayTracingInOneWeekend.html

namespace Engine
{

struct HitRecord;

class RaytracingMaterial
{
public:
    RaytracingMaterial(const char *type);

    const std::string &getType() const;

    virtual bool scatter(const Engine::Ray &r_in,
                         const HitRecord &rec,
                         Engine::Vector3 &attenuation,
                         Engine::Ray &scattered) const = 0;

private:
    std::string m_type;
};

class LambertianMaterial : public RaytracingMaterial
{
public:
    LambertianMaterial(const Engine::Vector3 &a);

    Engine::Vector3 &getAlbedo();

    virtual bool scatter(const Engine::Ray &r_in,
                         const HitRecord &rec,
                         Engine::Vector3 &attenuation,
                         Engine::Ray &scattered) const override;

private:
    Engine::Vector3 m_albedo;
};

class MetalMaterial : public RaytracingMaterial
{
public:
    MetalMaterial(const Vector3 &albedo, float f);

    Engine::Vector3 &getAlbedo();
    float &getFuzz();

    virtual bool scatter(const Engine::Ray &r_in,
                         const HitRecord &rec,
                         Engine::Vector3 &attenuation,
                         Engine::Ray &scattered) const override;

private:
    Engine::Vector3 m_albedo;
    float m_fuzz;
};

class DielectricMaterial : public RaytracingMaterial
{
public:
    DielectricMaterial(double indexOfRefraction);

    virtual bool scatter(const Engine::Ray &r_in,
                         const HitRecord &rec,
                         Engine::Vector3 &attenuation,
                         Engine::Ray &scattered) const override;

private:
    double m_ir;

    static double reflectance(double cosine, double refIdx);
};

} // namespace Engine

#endif