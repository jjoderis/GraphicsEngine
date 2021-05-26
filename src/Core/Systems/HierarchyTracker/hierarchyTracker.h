#ifndef ENGINE_CORE_SYSTEMS_HIERARCHYTRACKER
#define ENGINE_CORE_SYSTEMS_HIERARCHYTRACKER

#include "../../Components/Hierarchy/hierarchy.h"
#include "../../Components/Transform/transform.h"
#include "../../ECS/registry.h"

#include <map>

namespace Engine
{

namespace Systems
{

class HierarchyTracker
{
public:
    HierarchyTracker() = delete;
    HierarchyTracker(Registry &registry);

private:
    Registry &m_registry;

    // holds callback that gets called when a new hierarchy is added
    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<HierarchyComponent> hierarchy)>> m_AddHierarchyCB{};

    using meta_data = std::tuple<
        int, // the currently assigned parent node
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<HierarchyComponent>)>>, // callback for when the
                                                                                               // entity has a hierarchy
                                                                                               // update
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<HierarchyComponent>)>>, // callback for when the
                                                                                               // entity has its
                                                                                               // hierarchy removed
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>>, // callback for when the
                                                                                               // entity has a transform
                                                                                               // added or updated
        std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>>  // callback for when the
                                                                                               // entity has its
                                                                                               // transform removed
        >;

    std::map<unsigned int, meta_data> m_entityData{};

    // sets up tracking of tranform and hierarchy changes
    void associate(unsigned int entity);

    void handleTransformChanges(unsigned int entity);
    void awaitTransform(unsigned int entity);
    void removeTransform(unsigned int entity);

    // update transform based on parent
    void updateTransform(unsigned int entity, int parent);
    // update transforms of all children
    void updateChildren(unsigned int entity);
};

} // namespace Systems

} // namespace Engine

#endif