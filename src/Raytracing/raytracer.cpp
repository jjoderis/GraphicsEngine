#include "raytracer.h"

#include "../Core/Components/Camera/camera.h"
#include "../Core/Components/Geometry/geometry.h"
#include "../Core/Components/Light/light.h"
#include "../Core/Components/Transform/transform.h"
#include "../Core/ECS/registry.h"
#include "../Core/Util/Raycaster/raycaster.h"
#include "Components/Material/raytracingMaterial.h"

#include <thread>

Engine::Math::Vector4 calculateColor(Engine::Registry &registry, Engine::Util::Ray &ray);

void raytraceScenePart(Engine::Registry &registry,
                       std::vector<float> &texels,
                       int startX,
                       int startY,
                       int endX,
                       int endY,
                       int width,
                       int height);

std::vector<float> Engine::raytraceScene(Engine::Registry &registry, int width, int height)
{
    std::vector<float> pixelColors{};
    pixelColors.resize(3 * width * height, 0);

    Engine::Math::Vector4 color;

    auto possibleThreads = std::thread::hardware_concurrency();

    if (possibleThreads < 4)
    {
        raytraceScenePart(registry, pixelColors, 0, 0, width, height, width, height);
    }
    {
        possibleThreads = pow(2, floor(log2(possibleThreads)) - 1);

        int medianY = height / 2;

        int xSliceSize = width / possibleThreads;

        std::vector<std::thread> threads{};
        threads.reserve(possibleThreads * 2);

        int startX = 0;

        for (int i = 0; i < (possibleThreads - 1); ++i)
        {
            startX = i * xSliceSize;
            int endX = startX + xSliceSize;
            threads.emplace_back(std::thread(
                raytraceScenePart, std::ref(registry), std::ref(pixelColors), startX, 0, endX, medianY, width, height));
            threads.emplace_back(std::thread(raytraceScenePart,
                                             std::ref(registry),
                                             std::ref(pixelColors),
                                             startX,
                                             medianY,
                                             endX,
                                             height,
                                             width,
                                             height));
        }

        threads.emplace_back(std::thread(
            raytraceScenePart, std::ref(registry), std::ref(pixelColors), startX, 0, width, medianY, width, height));
        threads.emplace_back(std::thread(raytraceScenePart,
                                         std::ref(registry),
                                         std::ref(pixelColors),
                                         startX,
                                         medianY,
                                         width,
                                         height,
                                         width,
                                         height));

        for (int i = 0; i < threads.size(); ++i)
        {
            threads[i].join();
        }
    }

    return pixelColors;
}

void raytraceScenePart(Engine::Registry &registry,
                       std::vector<float> &texels,
                       int startX,
                       int startY,
                       int endX,
                       int endY,
                       int width,
                       int height)
{
    unsigned int activeCameraEntity = registry.getOwners<Engine::ActiveCameraComponent>()[0].front();
    auto camera = registry.getComponent<Engine::CameraComponent>(activeCameraEntity);

    Engine::Math::Vector4 color;

    for (int y = startY; y < endY; ++y)
    {
        for (int x = startX; x < endX; ++x)
        {
            Engine::Util::Ray cameraRay = camera->getCameraRay({x, y}, {width, height});

            color = calculateColor(registry, cameraRay);

            texels[3 * ((width * y) + x)] = color(0);
            texels[3 * ((width * y) + x) + 1] = color(1);
            texels[3 * ((width * y) + x) + 2] = color(2);
        }
    }
}

Engine::Math::Vector4 calculateLighting(Engine::Registry &registry,
                                        const Engine::Math::Vector4 &materialColor,
                                        const Engine::Util::RayIntersection &intersection);

Engine::Math::Vector4 calculateColor(Engine::Registry &registry, Engine::Util::Ray &ray)
{
    auto intersections = Engine::Util::castRay(ray, registry);

    if (!intersections.size())
    {
        return Engine::Math::Vector4{0, 0, 0, 0};
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

            auto surfaceNormal = ((1 - baryParams(0) - baryParams(1)) * vertexNormals[faces[intersectionFaceIndex]] +
                                  baryParams(0) * vertexNormals[faces[intersectionFaceIndex + 1]] +
                                  baryParams(1) * vertexNormals[faces[intersectionFaceIndex + 2]])
                                     .normalize();

            auto reflectedDirection = ray.getDirection().reflect(surfaceNormal);
            auto newOrigin =
                intersection.getIntersection() + reflectedDirection * std::numeric_limits<float>::epsilon();
            Engine::Util::Ray reflectedRay{newOrigin, reflectedDirection};
            return calculateColor(registry, reflectedRay);
        }
        else
        {
            return calculateLighting(registry, material->getColor(), intersection);
        }
    }

    return Engine::Math::Vector4{0.9, 0.126, 0.777, 1};
}

Engine::Math::Vector3 calculatePointLightColor(Engine::Registry &registry,
                                               unsigned int entity,
                                               const Engine::Util::RayIntersection &intersection);

Engine::Math::Vector4 calculateLighting(Engine::Registry &registry,
                                        const Engine::Math::Vector4 &materialColor,
                                        const Engine::Util::RayIntersection &intersection)
{
    Engine::Math::Vector4 color{0, 0, 0, 0};

    for (auto &pointLightOwners : registry.getOwners<Engine::PointLightComponent>())
    {
        for (auto owner : pointLightOwners)
        {
            Engine::Math::Vector3 lightColor = calculatePointLightColor(registry, owner, intersection);
            color += materialColor * Engine::Math::Vector4{lightColor, 1};
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

Engine::Math::Vector3 calculatePointLightColor(Engine::Registry &registry,
                                               unsigned int entity,
                                               const Engine::Util::RayIntersection &intersection)
{
    Engine::Math::Vector3 lighPosition{0, 0, 0};

    if (auto lightTransform = registry.getComponent<Engine::TransformComponent>(entity))
    {
        lighPosition = lightTransform->getModelMatrix() * Engine::Math::Vector4{lighPosition, 1};
    }

    unsigned int intersectionEntity = intersection.getEntity();
    int intersectionFaceIndex = intersection.getFace();

    auto geometry = registry.getComponent<Engine::GeometryComponent>(intersectionEntity);
    auto transform = registry.getComponent<Engine::TransformComponent>(intersectionEntity);

    auto baryParams = intersection.getBaryParams();

    auto &faces = geometry->getFaces();
    auto &vertexNormals = geometry->getNormals();

    auto surfaceNormal = (1 - baryParams(0) - baryParams(1)) * vertexNormals[faces[intersectionFaceIndex]] +
                         baryParams(0) * vertexNormals[faces[intersectionFaceIndex + 1]] +
                         baryParams(1) * vertexNormals[faces[intersectionFaceIndex + 2]];

    surfaceNormal = transform->getNormalMatrix() * Engine::Math::Vector4{surfaceNormal, 1};
    surfaceNormal.normalize();

    auto origin = intersection.getIntersection() + 10 * std::numeric_limits<float>::epsilon() * surfaceNormal;

    auto lightVector = lighPosition - intersection.getIntersection();
    float lightDist = lightVector.norm();
    lightVector /= lightDist;

    auto lightRay = Engine::Util::Ray(origin, lightVector);

    auto shadowIntersections = Engine::Util::castRay(lightRay, registry);

    if there
        was an intersection between the object and the light then it is in shadow if (shadowIntersections.size())
        {
            return Engine::Math::Vector4{0, 0, 0, 0};
        }

    float lightAngle = clamp(dot(lightVector, surfaceNormal), 0, 1);

    auto light = registry.getComponent<Engine::PointLightComponent>(entity);

    return lightAngle * light->getColor();
}