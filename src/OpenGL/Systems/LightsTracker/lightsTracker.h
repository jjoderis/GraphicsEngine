#ifndef ENGINE_OPENGL_SYSTEM_LIGHTSTRACKER
#define ENGINE_OPENGL_SYSTEM_LIGHTSTRACKER

#include "../../../Core/ECS/registry.h"
#include "../../../Core/Components/Light/light.h"
#include "../../../Core/Components/Transform/transform.h"
#include <glad/glad.h>
#include <map>
#include <tuple>

namespace Engine {
    namespace Systems {
        class OpenGLLightsTracker {
        private: 
            unsigned int& m_lightsUBO;

            int m_numLights{0};

            Registry& m_registry;

            // cb that sets up tracking of lights that get added 
            std::shared_ptr<std::function<void(unsigned int, LightComponent* light)>> m_AddLightCB{};

            using meta_data = std::tuple<
                size_t, // the offset at which the information for this entities light starts
                std::shared_ptr<std::function<void(unsigned int, LightComponent*)>>, // callback for when the entity has a light added or updated
                std::shared_ptr<std::function<void(unsigned int, LightComponent*)>>, // callback for when the entity has its light removed
                std::shared_ptr<std::function<void(unsigned int, TransformComponent*)>>, // callback for when the entity has a transform added or updated
                std::shared_ptr<std::function<void(unsigned int, TransformComponent*)>> // callback for when the entity has its transform removed
            >;

            std::map<unsigned int, meta_data> m_entityData{};

            void awaitTransform(unsigned int entity);
            void updateTransformInfo(unsigned int entity, TransformComponent* transform);
            void resetTransformInfo(unsigned int entity);

            void addLight(unsigned int entity, LightComponent* light);
            void removeLight(unsigned int entity);
        public:
            OpenGLLightsTracker() = delete;

            OpenGLLightsTracker(unsigned int& lightsUBO, Registry& registry);
        };
    }
}

#endif
