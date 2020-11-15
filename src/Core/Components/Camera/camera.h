#ifndef CORE_COMPONENTS_CAMERA
#define CORE_COMPONENTS_CAMERA

#include "../../Math/math.h"
#include "../../ECS/registry.h"
#include "../Transform/transform.h"

namespace Engine {
    // TODO: add actual implementation of perspective transformation

    class CameraComponent {
    private:
        // waits for the component to be added to an entity to then set up callbacks to react to changes in the Transform of that entity
        std::shared_ptr<std::function<void(unsigned int, CameraComponent*)>> m_associateCallback;
        // update view Matrix based on the changes in the Transform of the entity
        std::shared_ptr<std::function<void(unsigned int, TransformComponent*)>> m_transformUpdateCallback;
        // reset view matrix when the camera is removed from the related entity
        std::shared_ptr<std::function<void(unsigned int, TransformComponent*)>> m_removeTransformCallback;

        Math::Matrix4 m_viewMatrix{};
        Math::Matrix4 m_viewMatrixInverse{};
        Math::Matrix4 m_projectionMatrix{};
        Registry& m_registry;

        // sets up callbacks to react to the Transform of the entity changing
        void registerEntity(unsigned int entity);
        void setupUpdateCallback(unsigned int entity);
        void awaitTransformComponent(unsigned int entity);
        void update(unsigned int entity, TransformComponent* transform);
    public:
        CameraComponent() = delete;
        CameraComponent(Registry& registry);

        const Math::Matrix4& getViewMatrix();
        const Math::Matrix4& getViewMatrixInverse();
        const Math::Matrix4& getProjectionMatrix();
    };

}


#endif