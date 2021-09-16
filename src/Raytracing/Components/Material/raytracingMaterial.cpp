#include "raytracingMaterial.h"
#include "../../Util/hitrecord.h"

Engine::RaytracingMaterial::RaytracingMaterial(const char *type) : m_type{type} {}

const std::string &Engine::RaytracingMaterial::getType() const { return m_type; }

Engine::LambertianMaterial::LambertianMaterial(const Engine::Vector3 &a) : RaytracingMaterial("Lambertian"), m_albedo{a}
{
}

Engine::Vector3 &Engine::LambertianMaterial::getAlbedo() { return m_albedo; }

bool Engine::LambertianMaterial::scatter(const Engine::Ray &r_in,
                                         const HitRecord &rec,
                                         Engine::Vector3 &attenuation,
                                         Engine::Ray &scattered) const
{
    auto scatterDirection{rec.normal + randomUnitVector()};

    if (scatterDirection.nearZero())
    {
        scatterDirection = rec.normal;
    }

    scattered = Engine::Ray{rec.p, scatterDirection};
    attenuation = m_albedo;
    return true;
}

Engine::MetalMaterial::MetalMaterial(const Vector3 &albedo, float f)
    : RaytracingMaterial("Metal"), m_albedo{albedo}, m_fuzz{f < 1 ? f : 1}
{
}

Engine::Vector3 &Engine::MetalMaterial::getAlbedo() { return m_albedo; }
float &Engine::MetalMaterial::getFuzz() { return m_fuzz; }

bool Engine::MetalMaterial::scatter(const Engine::Ray &r_in,
                                    const HitRecord &rec,
                                    Engine::Vector3 &attenuation,
                                    Engine::Ray &scattered) const
{
    Vector3 reflected{reflect(normalize(r_in.getDirection()), rec.normal)};
    scattered = Engine::Ray(rec.p, reflected + m_fuzz * randomInUnitSphere());
    attenuation = m_albedo;
    return (dot(scattered.getDirection(), rec.normal) > 0);
}

Engine::DielectricMaterial::DielectricMaterial(double indexOfRefraction)
    : RaytracingMaterial("Dielectric"), m_ir{indexOfRefraction}
{
}

bool Engine::DielectricMaterial::scatter(const Engine::Ray &r_in,
                                         const HitRecord &rec,
                                         Engine::Vector3 &attenuation,
                                         Engine::Ray &scattered) const
{
    attenuation = Engine::Vector3{1.0, 1.0, 1.0};
    float refractionRatio{rec.frontFace ? (1.0 / m_ir) : m_ir};

    Engine::Vector3 unitDirection{normalize(r_in.getDirection())};
    float cosTheta{std::min(dot(-unitDirection, rec.normal), 1.0f)};
    float sinTheta{sqrt(1.0 - cosTheta * cosTheta)};

    bool cannotRefract{refractionRatio * sinTheta > 1.0};
    Engine::Vector3 direction{};

    if (cannotRefract || reflectance(cosTheta, refractionRatio) > MathLib::Util::random_number<float>())
    {
        direction = reflect(unitDirection, rec.normal);
    }
    else
    {
        direction = refract(unitDirection, rec.normal, refractionRatio);
    }

    scattered = Engine::Ray{rec.p, direction};
    return true;
}

double Engine::DielectricMaterial::reflectance(double cosine, double refIdx)
{
    auto r0{(1 - refIdx) / (1 + refIdx)};
    r0 = r0 * r0;
    return r0 + (1 - r0) * pow((1 - cosine), 5);
}
