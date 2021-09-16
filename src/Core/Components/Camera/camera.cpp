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

void Engine::CameraComponent::update()
{
    auto h{tan(m_fov / 2)};

    float viewportHeight{2.0 * h};
    float viewportWidth{m_aspect * viewportHeight};

    m_horizontal = Engine::Vector3{viewportWidth, 0, 0};
    m_vertical = Engine::Vector3{0, viewportHeight, 0};
    m_lowerLeftCorner = Engine::Point3{0, 0, 0} - m_horizontal / 2 - m_vertical / 2 - Engine::Vector3{0, 0, 1.0};
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
        m_projectionMatrix = Matrix4 {
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
        m_projectionMatrix = Matrix4 {
            c / m_aspect, 0.0f,                      0.0f,                                      0.0f,
                    0.0f,    c,                      0.0f,                                      0.0f,
                    0.0f, 0.0f,  -(m_far+m_near)/(m_far-m_near), -(2.0f*m_far*m_near)/(m_far-m_near),
                    0.0f, 0.0f,                           -1.0f,                                0.0f
        };
        // clang-format on
    }
}

const Engine::Matrix4 &Engine::CameraComponent::getProjectionMatrix() { return m_projectionMatrix; }

float &Engine::CameraComponent::getNear() { return m_near; }
void Engine::CameraComponent::setNear(float near)
{
    m_near = near;
    update();
}

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
void Engine::CameraComponent::setFov(float fov)
{
    m_fov = fov;
    update();
}

float &Engine::CameraComponent::getAspect() { return m_aspect; }
void Engine::CameraComponent::setAspect(float aspect)
{
    m_aspect = aspect;
    update();
}

bool Engine::CameraComponent::isPerspective() { return m_projection == ProjectionType::Perspective; }
bool Engine::CameraComponent::isOrtographic() { return m_projection == ProjectionType::Ortographic; }

Engine::Ray Engine::CameraComponent::getCameraSpaceRay(double u, double v) const
{
    return Engine::Ray{Engine::Point3{0, 0, 0},
                       m_lowerLeftCorner + u * m_horizontal + v * m_vertical - Engine::Point3{0, 0, 0}};
}

Engine::Ray Engine::CameraComponent::getCameraRay(double u, double v) const
{
    Engine::Ray ray{getCameraSpaceRay(u, v)};

    if (auto transform{m_registry.getComponent<Engine::TransformComponent>(m_entity)})
    {
        return transform->getViewMatrixWorldInverse() * ray;
    }
    else
    {
        return ray;
    }
}