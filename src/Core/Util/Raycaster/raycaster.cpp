#include "raycaster.h"

#include "../../Components/Geometry/geometry.h"
#include "../../Components/Render/render.h"
#include "../../Components/Transform/transform.h"
#include "../../ECS/registry.h"

Engine::Util::Ray::Ray(const Point3 &origin, const Vector3 &direction) : m_origin{origin}
{
    auto normalizedDirection{direction};
    m_direction = normalize(normalizedDirection);
}

const Engine::Point3 &Engine::Util::Ray::getOrigin() const { return m_origin; }
const Engine::Vector3 &Engine::Util::Ray::getDirection() const { return m_direction; }

Engine::Util::Ray Engine::Util::operator*(const Matrix4 &matrix, const Ray &ray)
{
    auto transformedOrigin{matrix * ray.getOrigin()};
    auto transformedDirection{matrix * ray.getDirection()};

    return Ray{transformedOrigin, transformedDirection};
}

Engine::Util::RayIntersection::RayIntersection(
    const Point3 &intersection, float distance, unsigned int entity, int face, const Vector2 &baryParams)
    : m_intersection{intersection}, m_distance{distance}, m_entity{entity}, m_face{face}, m_baryParams{baryParams}
{
}

bool Engine::Util::operator<(const RayIntersection &a, const RayIntersection &b)
{
    return a.getDistance() < b.getDistance();
}

float Engine::Util::RayIntersection::getDistance() const { return m_distance; }
unsigned int Engine::Util::RayIntersection::getEntity() const { return m_entity; }
const Engine::Point3 &Engine::Util::RayIntersection::getIntersection() const { return m_intersection; }
int Engine::Util::RayIntersection::getFace() const { return m_face; }
const Engine::Vector2 &Engine::Util::RayIntersection::getBaryParams() const { return m_baryParams; }

void calculateBoundingIntersection(Engine::Util::Ray &ray,
                                   Engine::AccelerationStructure &acc,
                                   Engine::GeometryComponent &geo,
                                   Engine::TransformComponent &transform,
                                   std::set<Engine::Util::RayIntersection> &intersections,
                                   unsigned int entity);

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
    Engine::Util::Ray transformedRay = transform->getMatrixWorldInverse() * ray;

    calculateBoundingIntersection(
        transformedRay, geometry->getAccStructure(), *geometry, *transform, intersections, entity);
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

void calculateTriangleIntersections(Engine::Util::Ray &ray,
                                    Engine::GeometryComponent &geo,
                                    std::vector<int> &triangles,
                                    Engine::TransformComponent &transform,
                                    std::set<Engine::Util::RayIntersection> &intersections,
                                    unsigned int entity);

// based on:
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
void calculateBoundingIntersection(Engine::Util::Ray &ray,
                                   Engine::AccelerationStructure &acc,
                                   Engine::GeometryComponent &geo,
                                   Engine::TransformComponent &transform,
                                   std::set<Engine::Util::RayIntersection> &intersections,
                                   unsigned int entity)
{
    auto origin = ray.getOrigin();
    auto direction = ray.getDirection();

    auto &min{acc.getMin()};
    auto &max{acc.getMax()};

    float t0x = (min.at(0) - origin.at(0)) / direction.at(0);
    float t1x = (max.at(0) - origin.at(0)) / direction.at(0);

    sortMinMax(t0x, t1x);

    float t0y = (min.at(1) - origin.at(1)) / direction.at(1);
    float t1y = (max.at(1) - origin.at(1)) / direction.at(1);

    sortMinMax(t0y, t1y);

    if (t0x > t1y || t0y > t1x)
    {
        return;
    }

    float tmin = (t0x > t0y) ? t0x : t0y;
    float tmax = (t1x < t1y) ? t1x : t1y;

    float t0z = (min.at(2) - origin.at(2)) / direction.at(2);
    float t1z = (max.at(2) - origin.at(2)) / direction.at(2);

    sortMinMax(t0z, t1z);

    if (tmin > t1z || t0z > tmax)
    {
        return;
    }

    calculateTriangleIntersections(ray, geo, acc.getTriangles(), transform, intersections, entity);

    for (auto &child : acc.getChildren())
    {
        calculateBoundingIntersection(ray, child, geo, transform, intersections, entity);
    }
}

void calculateTriangleIntersections(Engine::Util::Ray &ray,
                                    Engine::GeometryComponent &geo,
                                    std::vector<int> &triangles,
                                    Engine::TransformComponent &transform,
                                    std::set<Engine::Util::RayIntersection> &intersections,
                                    unsigned int entity)
{
    auto &faces = geo.getFaces();
    auto &vertices = geo.getVertices();

    const auto &origin{ray.getOrigin()};
    const auto &direction{ray.getDirection()};

    for (auto &startIndex : triangles)
    {
        auto &p0{vertices[faces[startIndex]]};
        auto &p1{vertices[faces[startIndex + 1]]};
        auto &p2{vertices[faces[startIndex + 2]]};
        auto e1{p1 - p0};
        auto e2{p2 - p0};
        auto s{origin - p0};

        float tripleProduct{1 / dot(cross(direction, e2), e1)};

        float t{tripleProduct * dot(cross(s, e1), e2)};
        float b1{tripleProduct * dot(cross(direction, e2), s)};
        float b2{tripleProduct * dot(cross(s, e1), direction)};

        // check if intersection with triangle plane is inside triangle and the triangle is in front of the ray
        if (b1 > 0 && b2 > 0 && b1 + b2 < 1 && t > 0)
        {
            auto intersection{affineCombination((1 - b1 - b2), p0, b1, p1, b2, p2)};

            intersection = transform.getMatrixWorld() * intersection;
            auto rayWorldOrigin{transform.getMatrixWorld() * origin};
            float distance{(intersection - rayWorldOrigin).norm()};

            intersections.emplace(
                Engine::Util::RayIntersection{intersection, distance, entity, startIndex, Engine::Vector2{b1, b2}});
        }
    }
}