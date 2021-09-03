#include "raytracer.h"

#include "../Core/Components/Camera/camera.h"
#include "../Core/Components/Geometry/geometry.h"
#include "../Core/Components/Light/light.h"
#include "../Core/Components/Transform/transform.h"
#include "../Core/ECS/registry.h"
#include "../Core/Util/Raycaster/raycaster.h"
#include "Components/Material/raytracingMaterial.h"

#include <thread>

Engine::Vector4 calculateColor(Engine::Registry &registry, Engine::Util::Ray &ray);

void raytraceScenePart(
    Engine::Registry &registry, std::vector<float> &texels, int start, int numTexels, int width, int height);

std::vector<float> Engine::raytraceScene(Engine::Registry &registry, int width, int height)
{
    std::vector<float> pixelColors{};
    pixelColors.resize(3 * width * height, 0);

    Engine::Vector4 color;

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

void raytraceScenePart(
    Engine::Registry &registry, std::vector<float> &texels, int start, int numTexels, int width, int height)
{
    unsigned int activeCameraEntity = registry.getOwners<Engine::ActiveCameraComponent>()[0].front();
    auto camera = registry.getComponent<Engine::CameraComponent>(activeCameraEntity);
    Engine::CameraComponent adjustedCamera{*camera};
    adjustedCamera.setAspect((float)width / (float)height);

    Engine::Vector4 color;

    for (int i{start}; i < start + numTexels; ++i)
    {
        int x{i % width};
        int y{i / width};

        Engine::Util::Ray cameraRay = adjustedCamera.getCameraRay({x, y}, {width, height});

        color = calculateColor(registry, cameraRay);

        texels[3 * i] = color(0);
        texels[3 * i + 1] = color(1);
        texels[3 * i + 2] = color(2);
    }
}

Engine::Vector4 calculateLighting(Engine::Registry &registry,
                                  const Engine::Vector4 &materialColor,
                                  const Engine::Util::RayIntersection &intersection);

Engine::Vector4 calculateColor(Engine::Registry &registry, Engine::Util::Ray &ray)
{
    auto intersections = Engine::Util::castRay(ray, registry);

    if (!intersections.size())
    {
        return Engine::Vector4{0, 0, 0, 0};
    }

    auto &intersection = *intersections.begin();

    unsigned int entity = intersection.getEntity();

    if (auto material = registry.getComponent<Engine::RaytracingMaterial>(entity))
    {
        if (material->isReflective())
        {
            auto intersection = *intersections.begin();
            auto intersectionEntity = intersection.getEntity();
            auto geometry = registry.getComponent<Engine::GeometryComponent>(intersectionEntity);
            auto transform = registry.getComponent<Engine::TransformComponent>(intersectionEntity);

            auto baryParams = intersection.getBaryParams();

            auto &faces = geometry->getFaces();
            auto &vertexNormals = geometry->getNormals();
            auto intersectionFaceIndex = intersection.getFace();

            auto surfaceNormal = affineCombination((1 - baryParams(0) - baryParams(1)),
                                                   vertexNormals[faces[intersectionFaceIndex]],
                                                   baryParams(0),
                                                   vertexNormals[faces[intersectionFaceIndex + 1]],
                                                   baryParams(1),
                                                   vertexNormals[faces[intersectionFaceIndex + 2]]);

            surfaceNormal = transform->getNormalMatrixWorld() * surfaceNormal;

            auto reflectedDirection = reflect(ray.getDirection(), surfaceNormal);
            auto newOrigin =
                intersection.getIntersection() + reflectedDirection * 10 * std::numeric_limits<float>::epsilon();
            Engine::Util::Ray reflectedRay{newOrigin, reflectedDirection};
            return calculateColor(registry, reflectedRay);
        }
        else
        {
            return calculateLighting(registry, material->getColor(), intersection);
        }
    }

    return Engine::Vector4{0.9, 0.126, 0.777, 1};
}

Engine::Vector4 calculatePointLightColor(Engine::Registry &registry,
                                         unsigned int entity,
                                         const Engine::Util::RayIntersection &intersection,
                                         const Engine::Vector4 &materialColor);

Engine::Vector4 calculateLighting(Engine::Registry &registry,
                                  const Engine::Vector4 &materialColor,
                                  const Engine::Util::RayIntersection &intersection)
{
    Engine::Vector4 color{0, 0, 0, 0};

    for (auto &pointLightOwners : registry.getOwners<Engine::PointLightComponent>())
    {
        for (auto owner : pointLightOwners)
        {
            color += calculatePointLightColor(registry, owner, intersection, materialColor);
        }
    }

    return color;
}

float clamp(float val, float min, float max)
{
    if (val < min)
    {
        return min;
    }

    if (val > max)
    {
        return max;
    }

    return val;
}

Engine::Vector4 calculatePointLightColor(Engine::Registry &registry,
                                         unsigned int entity,
                                         const Engine::Util::RayIntersection &intersection,
                                         const Engine::Vector4 &materialColor)
{
    Engine::Point3 lighPosition{0, 0, 0};

    if (auto lightTransform = registry.getComponent<Engine::TransformComponent>(entity))
    {
        lighPosition = lightTransform->getMatrixWorld() * lighPosition;
    }

    unsigned int intersectionEntity = intersection.getEntity();
    int intersectionFaceIndex = intersection.getFace();

    auto geometry = registry.getComponent<Engine::GeometryComponent>(intersectionEntity);
    auto transform = registry.getComponent<Engine::TransformComponent>(intersectionEntity);

    auto baryParams = intersection.getBaryParams();

    auto &faces = geometry->getFaces();
    auto &vertexNormals = geometry->getNormals();

    auto surfaceNormal = affineCombination((1 - baryParams(0) - baryParams(1)),
                                           vertexNormals[faces[intersectionFaceIndex]],
                                           baryParams(0),
                                           vertexNormals[faces[intersectionFaceIndex + 1]],
                                           baryParams(1),
                                           vertexNormals[faces[intersectionFaceIndex + 2]]);

    surfaceNormal = transform->getNormalMatrixWorld() * surfaceNormal;
    normalize(surfaceNormal);

    auto origin = intersection.getIntersection() + 10 * std::numeric_limits<float>::epsilon() * surfaceNormal;

    auto lightVector = lighPosition - intersection.getIntersection();
    float lightDist = lightVector.norm();
    lightVector /= lightDist;

    auto lightRay = Engine::Util::Ray(origin, lightVector);

    auto shadowIntersections = Engine::Util::castRay(lightRay, registry);

    // if there was an intersection between the object and the light then it is in shadow
    if (shadowIntersections.size() && shadowIntersections.begin()->getDistance() < lightDist)
    {
        return Engine::Vector4{0, 0, 0, 0};
    }

    float lightAngle = clamp(dot(lightVector, surfaceNormal), 0, 1);

    auto light = registry.getComponent<Engine::PointLightComponent>(entity);

    auto reflected{normalize(reflect(-lightVector, surfaceNormal))};

    auto activeCamera{registry.getOwners<Engine::ActiveCameraComponent>()[0].front()};
    auto cameraTransform{registry.getComponent<Engine::TransformComponent>(activeCamera)};
    auto cameraPosition{cameraTransform->getViewMatrixInverse() * Engine::Point3{0, 0, 0}};
    auto cameraDirection{normalize((cameraPosition - intersection.getIntersection()))};

    auto s{clamp(dot(cameraDirection, reflected), 0, 1)};

    s = std::max<float>(pow(s, 100), 0.0f);

    return lightAngle * Engine::Vector4{light->getColor(), 1} * materialColor + s * Engine::Vector4{1, 1, 1, 1};
}