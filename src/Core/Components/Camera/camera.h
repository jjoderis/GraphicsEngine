#ifndef CORE_COMPONENTS_CAMERA
#define CORE_COMPONENTS_CAMERA

#include "../../Math/math.h"
#include <functional>
#include <memory>

namespace Engine
{
class TransformComponent;
class Registry;

namespace Util
{
class Ray;
}

enum class ProjectionType
{
    Ortographic,
    Perspective
};
class CameraComponent
{
private:
    // waits for the component to be added to an entity to then set up callbacks
    // to react to changes in the Transform of that entity
    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<CameraComponent>)>> m_associateCallback;
    // update view Matrix based on the changes in the Transform of the entity
    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>> m_transformUpdateCallback;
    // reset view matrix when the camera is removed from the related entity
    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>> m_removeTransformCallback;

    Math::Matrix4 m_viewMatrix{};
    Math::Matrix4 m_viewMatrixInverse{};
    Math::Matrix4 m_projectionMatrix{};
    Registry &m_registry;

    float m_near{0.1f};
    float m_far{10.0f};
    float m_left{-1.0f};
    float m_right{1.0f};
    float m_bottom{-1.0f};
    float m_top{1.0f};
    float m_fov{M_PI_4};    // 45 degrees
    float m_aspect{16 / 9}; // use actual screen values here

    ProjectionType m_projection{ProjectionType::Perspective};

    // sets up callbacks to react to the Transform of the entity changing
    void registerEntity(unsigned int entity);
    void setupUpdateCallback(unsigned int entity);
    void awaitTransformComponent(unsigned int entity);
    void update(unsigned int entity, const std::shared_ptr<TransformComponent> &transform);

public:
    CameraComponent() = delete;
    CameraComponent(Registry &registry);

    void updateAspect(float aspect);
    void calculateProjection();

    const Math::Matrix4 &getViewMatrix();
    const Math::Matrix4 &getViewMatrixInverse();
    const Math::Matrix4 &getProjectionMatrix();

    float &getNear();
    void setNear(float near);

    float &getFar();
    void setFar(float far);

    float &getLeft();
    void setLeft(float left);

    float &getRight();
    void setRight(float right);

    float &getBottom();
    void setBottom(float bottom);

    float &getTop();
    void setTop(float top);

    float &getFov();
    void setFov(float fov);

    float &getAspect();
    void setAspect(float aspect);

    bool isPerspective();
    bool isOrtographic();

    Util::Ray getCameraRay(const Math::IVector2 &pixelPosition, const Math::IVector2 &screenSize);
    Util::Ray getCameraSpaceRay(const Math::IVector2 &pixelPosition, const Math::IVector2 &screenSize);
};

class ActiveCameraComponent
{
}; // The only point of this class is to keep track
   // through which camera we are currently looking
} // namespace Engine

#endif