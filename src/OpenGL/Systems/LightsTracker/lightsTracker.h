#ifndef ENGINE_OPENGL_SYSTEM_LIGHTSTRACKER
#define ENGINE_OPENGL_SYSTEM_LIGHTSTRACKER

#include <functional>
#include <glad/glad.h>
#include <map>
#include <memory>
#include <tuple>
namespace Engine
{
class Registry;
class TransformComponent;
class AmbientLightComponent;
class DirectionalLightComponent;
class PointLightComponent;
class SpotLightComponent;
namespace Systems
{
template <typename LightType>
class OpenGLLightsTracker
{
private:
    unsigned int &m_lightsUBO;

    int m_numLights{0};

    Registry &m_registry;

    // cb that sets up tracking of lights that get added
    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<LightType> light)>> m_AddLightCB{};

    using meta_data = std::tuple<
        size_t, // the offset at which the information for this entities light starts
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<LightType>)>>, // callback for when the entity
                                                                                      // has a light added or updated
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<LightType>)>>, // callback for when the entity
                                                                                      // has its light removed
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>>, // callback for when the
                                                                                               // entity has a transform
                                                                                               // added or updated
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>>  // callback for when the
                                                                                               // entity has its
                                                                                               // transform removed
        >;

    std::map<unsigned int, meta_data> m_entityData{};

    size_t getLightInfoSize();

    void awaitTransform(unsigned int entity);
    void updateTransformInfo(unsigned int entity, const std::shared_ptr<TransformComponent> &transform);
    void resetTransformInfo(unsigned int entity);

    void addLight(unsigned int entity, const std::shared_ptr<LightType> &light);
    void removeLight(unsigned int entity);
    void updateLightInfo(unsigned int entity, const std::shared_ptr<LightType> &light);

public:
    OpenGLLightsTracker() = delete;

    OpenGLLightsTracker(unsigned int &lightsUBO, Registry &registry);
};

using AmbientLightsTracker = OpenGLLightsTracker<AmbientLightComponent>;
using DirectionalLightsTracker = OpenGLLightsTracker<DirectionalLightComponent>;
using PointLightsTracker = OpenGLLightsTracker<PointLightComponent>;
using SpotLightsTracker = OpenGLLightsTracker<SpotLightComponent>;
} // namespace Systems
} // namespace Engine

#endif
