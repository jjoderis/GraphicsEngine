#include "hittable.h"

#include "../../../Core/Components/Transform/transform.h"
#include "../../../Core/ECS/registry.h"
#include "../../../Core/Util/Raycaster/raycaster.h"
#include "../../Util/hitrecord.h"
#include "../Material/raytracingMaterial.h"

auto baseColor{std::make_shared<Engine::LambertianMaterial>(Engine::Vector3{0.5, 0.5, 0})};

Engine::Hittable::Hittable(const char *type) : m_type{type} {}

const char *Engine::Hittable::getType() const { return m_type; }

Engine::HittableSphere::HittableSphere() : Engine::Hittable{"Sphere"} {}
Engine::HittableSphere::HittableSphere(const Engine::Point3 &cen, double r) : HittableSphere()
{
    m_center = cen;
    m_radius = r;
}

bool Engine::HittableSphere::hit(const Engine::Ray &r,
                                 Engine::Registry &registry,
                                 unsigned int entity,
                                 double t_min,
                                 double t_max,
                                 HitRecord &rec) const
{
    auto center{m_center};

    if (auto transform{registry.getComponent<Engine::TransformComponent>(entity)})
    {
        center = transform->getMatrixWorld() * center;
    }

    Engine::Vector3 oc{r.getOrigin() - center};
    auto a{r.getDirection().norm_squared()};
    auto half_b{dot(oc, r.getDirection())};
    auto c{oc.norm_squared() - m_radius * m_radius};

    auto discriminant{half_b * half_b - a * c};

    if (discriminant < 0)
    {
        return false;
    }

    auto sqrtd{sqrt(discriminant)};

    auto root{(-half_b - sqrtd) / a};

    if (root < t_min || t_max < root)
    {
        root = (-half_b + sqrtd) / a;
        if (root < t_min || t_max < root)
        {
            return false;
        }
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    auto outwardNormal{(rec.p - center) / m_radius};
    rec.setFaceNormal(r, outwardNormal);

    if (auto material{registry.getComponent<Engine::RaytracingMaterial>(entity)})
    {
        rec.matPtr = material;
    }
    else
    {
        rec.matPtr = baseColor;
    }

    return true;
}