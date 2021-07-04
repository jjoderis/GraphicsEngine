#include "raycaster.h"

#include "../../Components/BoundingBox/boundingBox.h"
#include "../../Components/Geometry/geometry.h"
#include "../../Components/Render/render.h"
#include "../../Components/Transform/transform.h"
#include "../../ECS/registry.h"

Engine::Util::Ray::Ray(const Math::Vector3 &origin, const Math::Vector3 &direction) : m_origin{origin}
{
    Math::Vector3 normalizedDirection = direction;
    m_direction = normalizedDirection.normalize();
}

Engine::Math::Vector3 Engine::Util::Ray::getOrigin() const { return m_origin; }
Engine::Math::Vector3 Engine::Util::Ray::getDirection() const { return m_direction; }

Engine::Util::Ray Engine::Util::operator*(const Math::Matrix4 &matrix, const Ray &ray)
{
    Math::Vector3 transformedOrigin = matrix * Math::Vector4{ray.getOrigin(), 1};
    Math::Vector3 transformedDirection = matrix * Math::Vector4{ray.getDirection(), 0};

    return Ray{transformedOrigin, transformedDirection};
}

Engine::Util::RayIntersection::RayIntersection(
    const Math::Vector3 &intersection, float distance, unsigned int entity, int face, const Math::Vector2 &baryParams)
    : m_intersection{intersection}, m_distance{distance}, m_entity{entity}, m_face{face}, m_baryParams{baryParams}
{
}

bool Engine::Util::operator<(const RayIntersection &a, const RayIntersection &b)
{
    return a.getDistance() < b.getDistance();
}

int Engine::Util::RayIntersection::getDistance() const { return m_distance; }
unsigned int Engine::Util::RayIntersection::getEntity() const { return m_entity; }
const Engine::Math::Vector3 &Engine::Util::RayIntersection::getIntersection() const { return m_intersection; }
int Engine::Util::RayIntersection::getFace() const { return m_face; }
const Engine::Math::Vector2 &Engine::Util::RayIntersection::getBaryParams() const { return m_baryParams; }

bool hasBoundingIntersection(unsigned int entity, Engine::Util::Ray &ray, Engine::Registry &registry);

void calculateGeometryIntersections(unsigned int entity,
                                    Engine::Util::Ray &ray,
                                    Engine::Registry &registry,
                                    std::set<Engine::Util::RayIntersection> &intersections);

std::set<Engine::Util::RayIntersection> Engine::Util::castRay(Engine::Util::Ray &ray, Registry &registry)
{
    // TODO: this will prevent non rendered elements from getting hit
    // the raycaster should be more general than rendering
    auto &renderableEntitiesLists = registry.getOwners<Engine::RenderComponent>();

    std::set<RayIntersection> intersections{};

    for (auto &entities : renderableEntitiesLists)
    {
        for (auto entity : entities)
        {
            // use bounding volume if available
            if (registry.hasComponent<Engine::BoundingBoxComponent>(entity) &&
                !hasBoundingIntersection(entity, ray, registry))
            {
                continue;
            }

            calculateGeometryIntersections(entity, ray, registry, intersections);
        }
    }

    return intersections;
}

// based on: https://www.youtube.com/watch?v=PI5jbAdT2zE
void calculateGeometryIntersections(unsigned int entity,
                                    Engine::Util::Ray &ray,
                                    Engine::Registry &registry,
                                    std::set<Engine::Util::RayIntersection> &intersections)
{
    auto geometry = registry.getComponent<Engine::GeometryComponent>(entity);
    auto transform = registry.getComponent<Engine::TransformComponent>(entity);

    // transform the ray into model space for following geometry comparisons
    Engine::Util::Ray transformedRay = transform->getModelMatrixInverse() * ray;
    Engine::Math::Vector3 origin = transformedRay.getOrigin();
    Engine::Math::Vector3 direction = transformedRay.getDirection();

    auto &faces = geometry->getFaces();
    auto &vertices = geometry->getVertices();
    if (faces.size())
    {
        for (int i = 0; i < faces.size(); i += 3)
        {
            Engine::Math::Vector3 p0{vertices[faces[i]]};
            Engine::Math::Vector3 p1{vertices[faces[i + 1]]};
            Engine::Math::Vector3 p2{vertices[faces[i + 2]]};
            Engine::Math::Vector3 e1{p1 - p0};
            Engine::Math::Vector3 e2{p2 - p0};
            Engine::Math::Vector3 s{origin - p0};

            float tripleProduct{1 / dot(cross(direction, e2), e1)};

            float t = tripleProduct * dot(cross(s, e1), e2);
            float b1 = tripleProduct * dot(cross(direction, e2), s);
            float b2 = tripleProduct * dot(cross(s, e1), direction);

            // check if intersection with triangle plane is inside triangle and the triangle is in front of the ray
            if (b1 > 0 && b2 > 0 && b1 + b2 < 1 && t > 0)
            {
                auto intersection = origin + t * direction;
                if (t < 0.0001)
                {
                    if (allClose(intersection, origin, 10 * std::numeric_limits<float>::epsilon()))
                    {
                        continue;
                    }
                }

                intersection = transform->getModelMatrix() * Engine::Math::Vector4{intersection, 1};
                float distance = (intersection - ray.getOrigin()).norm();

                intersections.emplace(
                    Engine::Util::RayIntersection{intersection, distance, entity, i, Engine::Math::Vector2{b1, b2}});
            }
        }
    }
}

void sortMinMax(float &a, float &b)
{
    if (a > b)
    {
        float tmp = a;
        a = b;
        b = tmp;
    }
}

// based on:
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
bool hasBoundingIntersection(unsigned int entity, Engine::Util::Ray &ray, Engine::Registry &registry)
{
    auto box = registry.getComponent<Engine::BoundingBoxComponent>(entity);
    auto transform = registry.getComponent<Engine::TransformComponent>(entity);

    // transform the ray into model space for following geometry comparisons
    Engine::Util::Ray transformedRay = transform->getModelMatrixInverse() * ray;
    Engine::Math::Vector3 origin = transformedRay.getOrigin();
    Engine::Math::Vector3 direction = transformedRay.getDirection();

    auto &min{box->getMin()};
    auto &max{box->getMax()};

    float t0x = (min.at(0) - origin.at(0)) / direction.at(0);
    float t1x = (max.at(0) - origin.at(0)) / direction.at(0);

    sortMinMax(t0x, t1x);

    float t0y = (min.at(1) - origin.at(1)) / direction.at(1);
    float t1y = (max.at(1) - origin.at(1)) / direction.at(1);

    sortMinMax(t0y, t1y);

    if (t0x > t1y || t0y > t1x)
    {
        return false;
    }

    float tmin = (t0x > t0y) ? t0x : t0y;
    float tmax = (t1x < t1y) ? t1x : t1y;

    float t0z = (min.at(2) - origin.at(2)) / direction.at(2);
    float t1z = (max.at(2) - origin.at(2)) / direction.at(2);

    sortMinMax(t0z, t1z);

    if (tmin > t1z || t0z > tmax)
    {
        return false;
    }

    return true;
}