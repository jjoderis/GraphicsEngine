#include "hierarchyTracker.h"

#include "../../Components/Hierarchy/hierarchy.h"
#include "../../Components/Transform/transform.h"
#include "../../ECS/registry.h"

Engine::Systems::HierarchyTracker::HierarchyTracker(Registry &registry) : m_registry{registry}
{
    m_AddHierarchyCB = m_registry.onAdded<HierarchyComponent>(
        [&](unsigned int entity, std::weak_ptr<HierarchyComponent> hierarchy)
        {
            // if the entity wasn't associated with this component before
            if (m_entityData.find(entity) == m_entityData.end())
            {
                m_entityData.emplace(
                    entity,
                    meta_data{-1,
                              std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<HierarchyComponent>)>>{},
                              std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<HierarchyComponent>)>>{},
                              std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>>{},
                              std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<TransformComponent>)>>{}});

                associate(entity);
            }
        });
}

/**
 * makes sure that an entity that is assigned a new parent doesn't change in appearance
 **/
void adjustTransform(Engine::Registry &registry, unsigned int entity, int newParent)
{
    auto transform{registry.getComponent<Engine::TransformComponent>(entity)};
    auto &oldWorldMatrix{transform->getMatrixWorld()};
    Engine::Matrix4 newParentWorldMat{};
    newParentWorldMat.setIdentity();
    Engine::Matrix4 newParentWorldInverseMat{};
    newParentWorldInverseMat.setIdentity();

    if (newParent > -1)
    {
        if (auto parentTrans{registry.getComponent<Engine::TransformComponent>(newParent)})
        {
            newParentWorldMat = parentTrans->getMatrixWorld();
            newParentWorldInverseMat = parentTrans->getMatrixWorldInverse();
        }
    }

    // calculate the elements new translation so it stays in the same position
    Engine::Point3 origin{0, 0, 0};
    auto test{newParentWorldInverseMat * oldWorldMatrix * origin};
    transform->setTranslation(test - Engine::Point3{0, 0, 0});

    // calculate the elements new rotation so it retains its orientation
    auto oldRot{Engine::extractEuler(oldWorldMatrix)};
    auto newRot{Engine::extractEuler(newParentWorldMat)};
    transform->setRotation(oldRot - newRot);

    // calculate the elements scaling factor so it keeps its size
    Engine::Vector3 e1{1, 0, 0};
    Engine::Vector3 e2{0, 1, 0};
    Engine::Vector3 e3{0, 0, 1};

    auto newE1{newParentWorldInverseMat * oldWorldMatrix * e1};
    auto newE2{newParentWorldInverseMat * oldWorldMatrix * e2};
    auto newE3{newParentWorldInverseMat * oldWorldMatrix * e3};

    transform->setScale({newE1.norm(), newE2.norm(), newE3.norm()});

    transform->update();
}

void updateWorldMatrices(
    Engine::Registry &registry, unsigned int entity, int parent, int oldParent, bool noUpdate = false)
{
    if (auto trans{registry.getComponent<Engine::TransformComponent>(entity)})
    {
        std::shared_ptr<Engine::TransformComponent> parentTrans{};

        if (oldParent != parent)
        {
            adjustTransform(registry, entity, parent);
        }

        if (parent > -1)
        {
            parentTrans = registry.getComponent<Engine::TransformComponent>(parent);
        }

        if (parent < 0 || !parentTrans)
        {
            trans->getMatrixWorld() = trans->getModelMatrix();
            trans->getMatrixWorldInverse() = trans->getModelMatrixInverse();
            trans->getNormalMatrixWorld() = trans->getNormalMatrix();
            trans->getNormalMatrixWorldInverse() = trans->getNormalMatrixInverse();
            trans->getViewMatrixWorld() = trans->getViewMatrix();
            trans->getViewMatrixWorldInverse() = trans->getViewMatrixInverse();
        }
        else
        {
            trans->getMatrixWorld() = parentTrans->getMatrixWorld() * trans->getModelMatrix();
            trans->getMatrixWorldInverse() = trans->getModelMatrixInverse() * parentTrans->getMatrixWorldInverse();

            trans->getNormalMatrixWorld() = parentTrans->getNormalMatrixWorld() * trans->getNormalMatrix();
            trans->getNormalMatrixWorldInverse() =
                trans->getNormalMatrixInverse() * parentTrans->getNormalMatrixWorldInverse();

            trans->getViewMatrixWorld() = trans->getViewMatrix() * parentTrans->getViewMatrixWorld();
            trans->getViewMatrixWorldInverse() =
                parentTrans->getViewMatrixWorldInverse() * trans->getViewMatrixInverse();
        }

        if (!noUpdate)
        {
            registry.updated<Engine::TransformComponent>(entity);
        }
    }
}

void Engine::Systems::HierarchyTracker::associate(unsigned int entity)
{
    meta_data &meta{m_entityData.at(entity)};

    // set hook for changes in Hierarchy
    std::get<1>(meta) = m_registry.onUpdate<HierarchyComponent>(
        entity,
        [&, entity](unsigned int updateEntity, std::weak_ptr<HierarchyComponent> hierarchyComp)
        {
            std::shared_ptr<HierarchyComponent> hierarchy{hierarchyComp.lock()};

            int currentParent = std::get<0>(meta);
            int newParent = hierarchy->getParent();

            // see if another parent was assigned
            if (newParent != currentParent)
            {
                std::shared_ptr<HierarchyComponent> parentHierarchy;

                // remove entity from old parents child list if it had a parent
                if (currentParent > -1)
                {
                    parentHierarchy = m_registry.getComponent<HierarchyComponent>(currentParent);
                    parentHierarchy->removeChild(updateEntity);
                }
                // add entity to child list of new parent
                if (newParent > -1)
                {
                    // ensure new parent to have a HierarchyComponent too
                    if (m_registry.hasComponent<HierarchyComponent>(newParent))
                    {
                        parentHierarchy = m_registry.getComponent<HierarchyComponent>(newParent);
                    }
                    else
                    {
                        parentHierarchy = m_registry.createComponent<HierarchyComponent>(newParent);
                    }

                    parentHierarchy->addChild(updateEntity);
                }

                std::get<0>(meta) = newParent;
                updateWorldMatrices(this->m_registry, entity, newParent, currentParent);
            }
        });

    // set hook for removal of hierarchy
    std::get<2>(meta) = m_registry.onRemove<HierarchyComponent>(
        [&, entity](unsigned int removeEntity, std::weak_ptr<HierarchyComponent> hierarchyComp)
        {
            if (entity == removeEntity)
            {
                int currentParent = std::get<0>(meta);

                if (currentParent > -1)
                {
                    std::shared_ptr<HierarchyComponent> parentHierarchy =
                        m_registry.getComponent<HierarchyComponent>(currentParent);

                    parentHierarchy->removeChild(entity);

                    updateWorldMatrices(this->m_registry, entity, -1, currentParent);
                }

                // remove as parent of each child
                for (unsigned int child : hierarchyComp.lock()->getChildren())
                {
                    m_registry.getComponent<HierarchyComponent>(child)->unsetParent();
                    m_registry.updated<HierarchyComponent>(child);
                }

                m_entityData.erase(entity);
            }
        });

    if (m_registry.hasComponent<TransformComponent>(entity))
    {
        handleTransformChanges(entity);
    }
    else
    {
        awaitTransform(entity);
    }
}

void Engine::Systems::HierarchyTracker::awaitTransform(unsigned int entity)
{
    meta_data &meta{m_entityData.at(entity)};

    // callback that gets called when a transform is added to the component
    std::get<3>(meta) = m_registry.onAdded<TransformComponent>(
        [&, entity](unsigned int addEntity, std::weak_ptr<TransformComponent> transComp)
        {
            if (addEntity == entity)
            {
                // calculate initial world matrix
                if (auto hierarchy{this->m_registry.getComponent<HierarchyComponent>(entity)})
                {
                    updateWorldMatrices(this->m_registry, entity, hierarchy->getParent(), hierarchy->getParent());
                }
                else
                {
                    updateWorldMatrices(this->m_registry, entity, -1, -1);
                }

                handleTransformChanges(entity);
            }
        });
}

void Engine::Systems::HierarchyTracker::handleTransformChanges(unsigned int entity)
{
    meta_data &meta{m_entityData.at(entity)};

    // callback that gets called when the transform gets changed
    std::get<3>(meta) = m_registry.onUpdate<TransformComponent>(
        entity,
        [&, entity](unsigned int updateEntity, std::weak_ptr<TransformComponent> transComp)
        {
            std::shared_ptr<HierarchyComponent> hierarchy{m_registry.getComponent<HierarchyComponent>(entity)};
            // update world matrix based on parent
            int parent = hierarchy->getParent();
            updateWorldMatrices(this->m_registry, entity, parent, parent, true);

            updateChildren(entity);
        });

    // callback that gets called when the transform is removed
    std::get<4>(meta) = m_registry.onRemove<TransformComponent>(
        [&, entity](unsigned int removeEntity, std::weak_ptr<TransformComponent> transComp)
        {
            if (entity == removeEntity)
            {
                updateChildren(entity);

                std::get<4>(meta).reset();
                awaitTransform(entity);
            }
        });
}

void Engine::Systems::HierarchyTracker::updateChildren(unsigned int entity)
{
    std::shared_ptr<HierarchyComponent> hierarchy{m_registry.getComponent<HierarchyComponent>(entity)};

    for (unsigned int child : hierarchy->getChildren())
    {
        updateWorldMatrices(m_registry, child, entity, entity);
    }
}