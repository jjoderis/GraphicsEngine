#include "camera.h"

#include "../../ECS/registry.h"
#include "../../Util/Raycaster/raycaster.h"
#include "../Transform/transform.h"

Engine::CameraComponent::CameraComponent(Registry &registry) : m_registry{registry}
{
    calculateProjection();

    m_associateCallback = m_registry.onAdded<CameraComponent>(
        [=](unsigned int entity, std::weak_ptr<CameraComponent> camera)
        {
            if (this == camera.lock().get())
            {
                this->m_entity = entity;
            }
        });
}

void Engine::CameraComponent::updateAspect(float aspect)
{
    m_aspect = aspect;
    calculateProjection();
}

void Engine::CameraComponent::calculateProjection()
{
    if (m_projection == ProjectionType::Ortographic)
    {
        // clang-format off
        m_projectionMatrix = Math::Matrix4 {
            2.0f / (m_right - m_left),                      0.0f,                    0.0f, -(m_right+m_left)/(m_right-m_left),
                                 0.0f, 2.0f / (m_top - m_bottom),                    0.0f, -(m_top+m_bottom)/(m_top-m_bottom),
                                 0.0f,                      0.0f, 2.0f / (m_far - m_near),     -(m_far+m_near)/(m_far-m_near),
                                 0.0f,                      0.0f,                    0.0f,                               1.0f
        };
        // clang-format on
    }
    else
    {
        float c{1.0f / tan(m_fov / 2)};

        // clang-format off
        m_projectionMatrix = Math::Matrix4 {
            c / m_aspect, 0.0f,                      0.0f,                                      0.0f,
                    0.0f,    c,                      0.0f,                                      0.0f,
                    0.0f, 0.0f,  -(m_far+m_near)/(m_far-m_near), -(2.0f*m_far*m_near)/(m_far-m_near),
                    0.0f, 0.0f,                           -1.0f,                                0.0f
        };
        // clang-format on
    }
}

const Engine::Math::Matrix4 &Engine::CameraComponent::getProjectionMatrix() { return m_projectionMatrix; }

float &Engine::CameraComponent::getNear() { return m_near; }
void Engine::CameraComponent::setNear(float near) { m_near = near; }

float &Engine::CameraComponent::getFar() { return m_far; }
void Engine::CameraComponent::setFar(float far) { m_far = far; }

float &Engine::CameraComponent::getLeft() { return m_left; }
void Engine::CameraComponent::setLeft(float left) { m_left = left; }

float &Engine::CameraComponent::getRight() { return m_right; }
void Engine::CameraComponent::setRight(float right) { m_right = right; }

float &Engine::CameraComponent::getBottom() { return m_bottom; }
void Engine::CameraComponent::setBottom(float bottom) { m_bottom = bottom; }

float &Engine::CameraComponent::getTop() { return m_top; }
void Engine::CameraComponent::setTop(float top) { m_top = top; }

float &Engine::CameraComponent::getFov() { return m_fov; }
void Engine::CameraComponent::setFov(float fov) { m_fov = fov; }

float &Engine::CameraComponent::getAspect() { return m_aspect; }
void Engine::CameraComponent::setAspect(float aspect) { m_aspect = aspect; }

bool Engine::CameraComponent::isPerspective() { return m_projection == ProjectionType::Perspective; }
bool Engine::CameraComponent::isOrtographic() { return m_projection == ProjectionType::Ortographic; }

Engine::Util::Ray Engine::CameraComponent::getCameraSpaceRay(const Math::IVector2 &pixelPosition,
                                                             const Math::IVector2 &screenSize)
{
    float normalizedX = 2 * ((pixelPosition(0) + 0.5) / screenSize(0)) - 1;
    float normalizedY = 1 - 2 * ((pixelPosition(1) + 0.5) / screenSize(1));

    float projectionPlaneWidth = tan(m_fov / 2);

    float cameraX = projectionPlaneWidth * m_aspect * normalizedX;
    float cameraY = projectionPlaneWidth * normalizedY;

    return {{0, 0, 0}, {cameraX, cameraY, -1}};
}

Engine::Util::Ray Engine::CameraComponent::getCameraRay(const Math::IVector2 &pixelPosition,
                                                        const Math::IVector2 &screenSize)
{
    Engine::Util::Ray ray{getCameraSpaceRay(pixelPosition, screenSize)};

    if (auto transform{m_registry.getComponent<Engine::TransformComponent>(m_entity)})
    {
        return transform->getViewMatrixWorldInverse() * ray;
    }
    else
    {
        return ray;
    }
}