#include "raytracer.h"

#include "../Core/Components/Camera/camera.h"
#include "../Core/Components/Geometry/geometry.h"
#include "../Core/Components/Light/light.h"
#include "../Core/Components/Transform/transform.h"
#include "../Core/ECS/registry.h"
#include "Components/Hittable/hittable.h"
#include "Components/Material/raytracingMaterial.h"
#include "Util/hitrecord.h"

#include <thread>

// Code from Ray Tracing in One Weekend, adapted to fit into our ECS System:
// https://raytracing.github.io/books/RayTracingInOneWeekend.html

constexpr float pi{3.1415926535897932385};

Engine::Vector4 calculateColor(Engine::Registry &registry, Engine::Ray &ray);

void raytraceScenePart(
    Engine::Registry &registry, std::vector<float> &texels, int start, int numTexels, int width, int height);

std::vector<float> Engine::raytraceScene(Engine::Registry &registry, int width, int height)
{
    std::vector<float> pixelColors{};
    pixelColors.resize(3 * width * height, 0);

    auto possibleThreads = std::thread::hardware_concurrency();

    std::vector<std::thread> threads{};
    threads.reserve(possibleThreads);

    int texelsPerThread{(width * height) / possibleThreads};
    int currentOffset{0};
    for (int i{0}; i < possibleThreads - 1; ++i)
    {
        threads.emplace_back(std::thread(raytraceScenePart,
                                         std::ref(registry),
                                         std::ref(pixelColors),
                                         currentOffset,
                                         texelsPerThread,
                                         width,
                                         height));
        currentOffset += texelsPerThread;
    }

    threads.emplace_back(std::thread(raytraceScenePart,
                                     std::ref(registry),
                                     std::ref(pixelColors),
                                     currentOffset,
                                     (width * height) - currentOffset,
                                     width,
                                     height));

    for (int i = 0; i < threads.size(); ++i)
    {
        threads[i].join();
    }

    return pixelColors;
}

Engine::Vector3 rayColor(const Engine::Ray &r, Engine::Registry &registry, int depth);

void raytraceScenePart(
    Engine::Registry &registry, std::vector<float> &texels, int start, int numTexels, int width, int height)
{
    unsigned int activeCameraEntity = registry.getOwners<Engine::ActiveCameraComponent>()[0].front();
    auto camera = registry.getComponent<Engine::CameraComponent>(activeCameraEntity);
    Engine::CameraComponent adjustedCamera{*camera};
    adjustedCamera.setAspect((float)width / (float)height);
    adjustedCamera.update();

    Engine::Vector3 color{0.0, 0.0, 0.0};

    for (int i{start}; i < start + numTexels; ++i)
    {
        int x{i % width};
        int y{height - (i / width)};

        int samplesPerPixel = 100;
        int maxDepth = 50;

        for (int s{0}; s < samplesPerPixel; ++s)
        {
            auto u = (x + MathLib::Util::random_number<float>()) / (width - 1);
            auto v = (y + MathLib::Util::random_number<float>()) / (height - 1);
            Engine::Ray cameraRay = adjustedCamera.getCameraRay(u, v);

            color += rayColor(cameraRay, registry, maxDepth);
        }

        color = sqrtVP(color / samplesPerPixel);

        texels[3 * i] = color(0);
        texels[3 * i + 1] = color(1);
        texels[3 * i + 2] = color(2);
    }
}

bool intersectScene(Engine::Registry &registry, const Engine::Ray &r, Engine::HitRecord &rec)
{
    const auto &hittables{registry.getOwners<Engine::Hittable>()};

    Engine::HitRecord tempRec;
    bool hitAnything{false};
    auto closestSoFar = std::numeric_limits<float>::infinity();

    for (auto &entityList : hittables)
    {
        for (auto entity : entityList)
        {
            if (registry.getComponent<Engine::Hittable>(entity)->hit(r, registry, entity, 0.001, closestSoFar, tempRec))
            {
                hitAnything = true;
                closestSoFar = tempRec.t;
                rec = tempRec;
            }
        }
    }

    return hitAnything;
}

Engine::Vector3 rayColor(const Engine::Ray &r, Engine::Registry &registry, int depth)
{
    // Engine::HitRecord rec{};

    // if (depth <= 0)
    // {
    //     return {0, 0, 0};
    // }

    // if (intersectScene(registry, r, rec))
    // {
    //     Engine::Ray scattered{{0, 0, 0}, {0, 0, 0}};
    //     Engine::Vector3 attenuation;
    //     if (rec.matPtr->scatter(r, rec, attenuation, scattered))
    //     {
    //         return attenuation * rayColor(scattered, registry, depth - 1);
    //     }
    //     return Engine::Vector3{0, 0, 0};
    // }

    // auto t{0.5 * (r.getDirection()(1) + 1.0)};
    // return (1.0 - t) * Engine::Vector3{1.0, 1.0, 1.0} + t * Engine::Vector3{0.5, 0.7, 1.0};

    Engine::HitRecord rec{};

    Engine::Vector3 currentColor(1, 1, 1);
    Engine::Ray currentRay{r};
    Engine::Ray nextRay{{0, 0, 0}, {0, 0, 0}};

    for (int i{0}; i < depth; ++i)
    {
        if (intersectScene(registry, currentRay, rec))
        {
            Engine::Vector3 attenuation;
            if (rec.matPtr->scatter(currentRay, rec, attenuation, nextRay))
            {
                currentColor = currentColor * attenuation;
                currentRay = nextRay;
            }
            else
            {
                return Engine::Vector3{0, 0, 0};
            }
        }
        else
        {
            Engine::Vector3 unit_direction = normalize(currentRay.getDirection());
            auto t = 0.5 * (unit_direction(1) + 1.0);
            return currentColor * ((1.0 - t) * Engine::Vector3(1.0, 1.0, 1.0) + t * Engine::Vector3(0.5, 0.7, 1.0));
        }
    }

    return Engine::Vector3{0, 0, 0};
}

// Engine::Vector4 calculateLighting(Engine::Registry &registry,
//                                   const Engine::Vector4 &materialColor,
//                                   const Engine::Util::RayIntersection &intersection);

// Engine::Vector4 calculateColor(Engine::Registry &registry, Engine::Util::Ray &ray)
// {
//     auto intersections = Engine::Util::castRay(ray, registry);

//     if (!intersections.size())
//     {
//         return Engine::Vector4{0, 0, 0, 0};
//     }

//     auto &intersection = *intersections.begin();

//     unsigned int entity = intersection.getEntity();

//     if (auto material = registry.getComponent<Engine::RaytracingMaterial>(entity))
//     {
//         if (material->isReflective())
//         {
//             auto intersection = *intersections.begin();
//             auto intersectionEntity = intersection.getEntity();
//             auto geometry = registry.getComponent<Engine::GeometryComponent>(intersectionEntity);
//             auto transform = registry.getComponent<Engine::TransformComponent>(intersectionEntity);

//             auto baryParams = intersection.getBaryParams();

//             auto &faces = geometry->getFaces();
//             auto &vertexNormals = geometry->getNormals();
//             auto intersectionFaceIndex = intersection.getFace();

//             auto surfaceNormal = affineCombination((1 - baryParams(0) - baryParams(1)),
//                                                    vertexNormals[faces[intersectionFaceIndex]],
//                                                    baryParams(0),
//                                                    vertexNormals[faces[intersectionFaceIndex + 1]],
//                                                    baryParams(1),
//                                                    vertexNormals[faces[intersectionFaceIndex + 2]]);

//             surfaceNormal = transform->getNormalMatrixWorld() * surfaceNormal;

//             auto reflectedDirection = reflect(ray.getDirection(), surfaceNormal);
//             auto newOrigin =
//                 intersection.getIntersection() + reflectedDirection * 10 * std::numeric_limits<float>::epsilon();
//             Engine::Ray reflectedRay{newOrigin, reflectedDirection};
//             return calculateColor(registry, reflectedRay);
//         }
//         else
//         {
//             return calculateLighting(registry, material->getColor(), intersection);
//         }
//     }

//     return Engine::Vector4{0.9, 0.126, 0.777, 1};
// }

// Engine::Vector4 calculatePointLightColor(Engine::Registry &registry,
//                                          unsigned int entity,
//                                          const Engine::Util::RayIntersection &intersection,
//                                          const Engine::Vector4 &materialColor);

// Engine::Vector4 calculateLighting(Engine::Registry &registry,
//                                   const Engine::Vector4 &materialColor,
//                                   const Engine::Util::RayIntersection &intersection)
// {
//     Engine::Vector4 color{0, 0, 0, 0};

//     for (auto &pointLightOwners : registry.getOwners<Engine::PointLightComponent>())
//     {
//         for (auto owner : pointLightOwners)
//         {
//             color += calculatePointLightColor(registry, owner, intersection, materialColor);
//         }
//     }

//     return color;
// }

// float clamp(float val, float min, float max)
// {
//     if (val < min)
//     {
//         return min;
//     }

//     if (val > max)
//     {
//         return max;
//     }

//     return val;
// }

// Engine::Vector4 calculatePointLightColor(Engine::Registry &registry,
//                                          unsigned int entity,
//                                          const Engine::Util::RayIntersection &intersection,
//                                          const Engine::Vector4 &materialColor)
// {
//     Engine::Point3 lighPosition{0, 0, 0};

//     if (auto lightTransform = registry.getComponent<Engine::TransformComponent>(entity))
//     {
//         lighPosition = lightTransform->getMatrixWorld() * lighPosition;
//     }

//     unsigned int intersectionEntity = intersection.getEntity();
//     int intersectionFaceIndex = intersection.getFace();

//     auto geometry = registry.getComponent<Engine::GeometryComponent>(intersectionEntity);
//     auto transform = registry.getComponent<Engine::TransformComponent>(intersectionEntity);

//     auto baryParams = intersection.getBaryParams();

//     auto &faces = geometry->getFaces();
//     auto &vertexNormals = geometry->getNormals();

//     auto surfaceNormal = affineCombination((1 - baryParams(0) - baryParams(1)),
//                                            vertexNormals[faces[intersectionFaceIndex]],
//                                            baryParams(0),
//                                            vertexNormals[faces[intersectionFaceIndex + 1]],
//                                            baryParams(1),
//                                            vertexNormals[faces[intersectionFaceIndex + 2]]);

//     surfaceNormal = transform->getNormalMatrixWorld() * surfaceNormal;
//     normalize(surfaceNormal);

//     auto origin = intersection.getIntersection() + 10 * std::numeric_limits<float>::epsilon() * surfaceNormal;

//     auto lightVector = lighPosition - intersection.getIntersection();
//     float lightDist = lightVector.norm();
//     lightVector /= lightDist;

//     auto lightRay = Engine::Ray(origin, lightVector);

//     auto shadowIntersections = Engine::Util::castRay(lightRay, registry);

//     // if there was an intersection between the object and the light then it is in shadow
//     if (shadowIntersections.size() && shadowIntersections.begin()->getDistance() < lightDist)
//     {
//         return Engine::Vector4{0, 0, 0, 0};
//     }

//     float lightAngle = clamp(dot(lightVector, surfaceNormal), 0, 1);

//     auto light = registry.getComponent<Engine::PointLightComponent>(entity);

//     auto reflected{normalize(reflect(-lightVector, surfaceNormal))};

//     auto activeCamera{registry.getOwners<Engine::ActiveCameraComponent>()[0].front()};
//     auto cameraTransform{registry.getComponent<Engine::TransformComponent>(activeCamera)};
//     auto cameraPosition{cameraTransform->getViewMatrixInverse() * Engine::Point3{0, 0, 0}};
//     auto cameraDirection{normalize((cameraPosition - intersection.getIntersection()))};

//     auto s{clamp(dot(cameraDirection, reflected), 0, 1)};

//     s = std::max<float>(pow(s, 100), 0.0f);

//     return lightAngle * Engine::Vector4{light->getColor(), 1} * materialColor + s * Engine::Vector4{1, 1, 1, 1};
// }