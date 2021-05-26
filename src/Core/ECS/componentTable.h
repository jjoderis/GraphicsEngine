#ifndef CORE_ECS_COMPONENTTABLE
#define CORE_ECS_COMPONENTTABLE

#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <vector>

namespace Engine
{
template <typename ComponentType>
struct ComponentTable
{
private:
    std::vector<int> m_sparse;
    std::vector<std::shared_ptr<ComponentType>> m_components{};
    std::vector<std::list<unsigned int>> m_owners{};

    // the implementation of the callback system is based on this blog post:
    // http://nercury.github.io/c++/interesting/2016/02/22/weak_ptr-and-event-cleanup.html
    using component_table_callback = std::function<void(unsigned int, std::weak_ptr<ComponentType>)>;
    // callbacks that are called after a component was added to an entity
    std::list<std::weak_ptr<component_table_callback>> m_addCallbacks{};
    // callbacks that are called before a component is removed from an entity
    std::list<std::weak_ptr<component_table_callback>> m_removeCallbacks{};
    // callbacks for the updates of a specific component
    std::vector<std::list<std::weak_ptr<component_table_callback>>> m_updateCallbacks{};

    //  Add component only if it does not exit: return index
    int ensureComponent(std::weak_ptr<ComponentType> component)
    {
        for (unsigned int i = 0; i < m_components.size(); ++i)
        {
            if (m_components[i] == component.lock())
            {
                return i;
            }
        }

        m_components.push_back(component.lock());
        m_owners.push_back(std::list<unsigned int>{});
        m_updateCallbacks.push_back(std::list<std::weak_ptr<component_table_callback>>{});
        return m_components.size() - 1;
    }

    void ensureEntity(unsigned int entityId)
    {
        if (entityId >= m_sparse.size())
        {
            m_sparse.resize(entityId + 1, -1);
        }
    }

    void invokeAndCleanup(std::list<std::weak_ptr<component_table_callback>> &cbs,
                          unsigned int entityId,
                          std::weak_ptr<ComponentType> component)
    {
        // invoke all callbacks that are still valid
        for (std::weak_ptr<component_table_callback> &cb : cbs)
        {
            if (std::shared_ptr<component_table_callback> callback = cb.lock())
            {
                (*callback)(entityId, component);
            }
        }

        // remove all invalid callbacks
        cbs.erase(std::remove_if(
                      cbs.begin(), cbs.end(), [](std::weak_ptr<component_table_callback> cb) { return cb.expired(); }),
                  cbs.end());
    }

public:
    ComponentTable(unsigned int numEntities) { m_sparse = std::vector<int>(numEntities, -1); }

    std::shared_ptr<ComponentType> addComponent(unsigned int entityId, std::weak_ptr<ComponentType> component)
    {
        ensureEntity(entityId);
        int componentIndex = ensureComponent(component);

        // entity has a component
        if (m_sparse[entityId] != -1)
        {
            // if the entity already points to the component
            if (m_components[m_sparse[entityId]] == component.lock())
            {
                return m_components[m_sparse[entityId]];
            }

            // if the entity points to another component
            removeComponent(entityId);
        }

        m_sparse[entityId] = componentIndex;
        m_owners[componentIndex].push_back(entityId);

        invokeAndCleanup(m_addCallbacks, entityId, m_components[componentIndex]);

        return m_components[m_sparse[entityId]];
    }

    void removeComponent(unsigned int entityId)
    {
        ensureEntity(entityId);
        // entity has no component of this type
        if (m_sparse[entityId] == -1)
        {
            return;
        }

        unsigned int componentIndex = m_sparse[entityId];

        invokeAndCleanup(m_removeCallbacks, entityId, m_components[componentIndex]);

        // remove entity from owner list
        std::list<unsigned int> &allOwners = m_owners[componentIndex];
        allOwners.remove(entityId);

        // remove component if there are no owners
        if (allOwners.size() == 0)
        {
            auto it = m_components.begin();
            // decrease the pointed to index of all entities which point to a
            // following component
            auto ownerIt = m_owners.begin();
            for (auto following{ownerIt + componentIndex + 1}; following != m_owners.end(); ++following)
            {
                for (unsigned int owner : *following)
                {
                    m_sparse[owner]--;
                }
            }

            m_components.erase(it + componentIndex);
            m_owners.erase(m_owners.begin() + componentIndex);
            m_updateCallbacks.erase(m_updateCallbacks.begin() + componentIndex);
        }

        m_sparse[entityId] = -1;
    }

    bool hasComponent(unsigned int entityId)
    {
        ensureEntity(entityId);
        return m_sparse[entityId] != -1;
    }

    std::shared_ptr<ComponentType> getComponent(unsigned int entityId)
    {
        ensureEntity(entityId);

        if (m_sparse[entityId] == -1)
        {
            return nullptr;
        }

        return m_components[m_sparse[entityId]];
    }

    std::vector<std::shared_ptr<ComponentType>> getComponents() { return m_components; }

    std::vector<std::list<unsigned int>> getOwners() { return m_owners; }

    std::list<unsigned int> getOwners(std::weak_ptr<ComponentType> component)
    {
        for (unsigned int i{0}; i < m_owners.size(); ++i)
        {
            if (m_components[i] == component.lock())
            {
                return m_owners[i];
            }
        }

        return std::list<unsigned int>{};
    }

    std::shared_ptr<component_table_callback> onAdded(component_table_callback &&cb)
    {
        std::shared_ptr<component_table_callback> shared = std::make_shared<component_table_callback>(cb);
        m_addCallbacks.push_back(shared);
        return shared;
    }

    std::shared_ptr<component_table_callback> onRemove(component_table_callback &&cb)
    {
        std::shared_ptr<component_table_callback> shared = std::make_shared<component_table_callback>(cb);
        m_removeCallbacks.push_back(shared);
        return shared;
    }

    std::shared_ptr<component_table_callback> onUpdate(unsigned int entityId, component_table_callback &&cb)
    {
        if (m_sparse[entityId] > -1)
        {
            std::shared_ptr<component_table_callback> shared = std::make_shared<component_table_callback>(cb);
            m_updateCallbacks[m_sparse[entityId]].push_back(shared);
            return shared;
        }

        return std::shared_ptr<component_table_callback>{};
    }

    // called after one updated a coponent of an entity to signal the update to
    // everyone who added a callback
    void updated(unsigned int entityId)
    {
        int componentIndex = m_sparse[entityId];
        if (componentIndex > -1)
        {
            // There was an error where the cb list changed while being inside invokeAndCleanup
            std::list<std::weak_ptr<component_table_callback>> cbs{m_updateCallbacks[componentIndex]};
            invokeAndCleanup(cbs, entityId, m_components[componentIndex]);
            m_updateCallbacks[componentIndex] = cbs;
        }
    }
};
} // namespace Engine

#endif