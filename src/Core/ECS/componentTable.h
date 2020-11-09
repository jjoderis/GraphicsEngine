#ifndef CORE_ECS_COMPONENTTABLE
#define CORE_ECS_COMPONENTTABLE

#include <vector>
#include <list>
#include <memory>
#include <functional>
#include <algorithm>

namespace Engine {
    template<typename ComponentType>
    struct ComponentTable {
    private:
        std::vector<int> m_sparse;
        std::vector<std::unique_ptr<ComponentType>> m_components{};
        std::vector<std::list<unsigned int>> m_owners{};
        // TODO: list containing callbacks for remove (per entity); list containing callbacks for update (per component); functions to add and remove callbacks; list containing callbacks for add (per entity)
        // why?: ex: RenderComponent we need to resize buffers when a Material or Geometry isn't associated with a owner of the RenderComponent anymore
        // we need to update the Buffers when the value of a component changes
        // maybe general list for add and remove (for all entities) : for RenderComponent always call associate with the entity and component maybe void(unsigned int entity, ComponentType* component)
        

        // the implementation of the callback system is loosely based on this blog post: http://nercury.github.io/c++/interesting/2016/02/22/weak_ptr-and-event-cleanup.html
        using component_table_callback = std::function<void(unsigned int, ComponentType*)>;
        // callbacks that are called after a component was added to an entity
        std::list<std::weak_ptr<component_table_callback>> m_addCallbacks{};
        // callbacks that are called before a component is removed from an entity
        std::list<std::weak_ptr<component_table_callback>> m_removeCallbacks{};
        // callbacks for the updates of a specific component
        std::vector<std::list<std::weak_ptr<component_table_callback>>> m_updateCallbacks{};

        //  Add component only if it does not exit: return index
        int ensureComponent(ComponentType* component)
        {
            for (unsigned int i = 0; i < m_components.size(); ++i)
            {
                if (m_components[i].get() == component)
                {
                    return i;
                }
            }

            m_components.push_back(std::unique_ptr<ComponentType>{component});
            m_owners.push_back(std::list<unsigned int>{});
            m_updateCallbacks.push_back(std::list<std::weak_ptr<component_table_callback>>{});
            return m_components.size() - 1;
        }

        void ensureEntity(unsigned int entityId)
        {
            if (entityId >= m_sparse.size())
            {
                m_sparse.resize(entityId+1, -1);
            }
        }

        void invokeAndCleanup(std::list<std::weak_ptr<component_table_callback>> &cbs, unsigned int entityId, ComponentType* component) {
            // invoke all callbacks that are still valid
            for (std::weak_ptr<component_table_callback>& cb : cbs) {
                if (std::shared_ptr<component_table_callback> callback = cb.lock()) {
                    (* callback)(entityId, component);
                }
            }

            // remove all invalid callbacks
            cbs.erase(std::remove_if(cbs.begin(), cbs.end(), [] (std::weak_ptr<component_table_callback> cb) {
                return cb.expired();
            }), cbs.end());
        }

    public: 
        ComponentTable(unsigned int numEntities)
        {
            m_sparse = std::vector<int>(numEntities, -1);
        }

        ComponentType* addComponent(unsigned int entityId, ComponentType* component)
        {
            ensureEntity(entityId);
            int componentIndex = ensureComponent(component);

            // entity has a component
            if (m_sparse[entityId] != -1)
            {
                // if the entity already points to the component
                if (m_components[m_sparse[entityId]].get() == component)
                {
                    return component;
                }

                // if the entity points to another component
                removeComponent(entityId);
            }

            m_sparse[entityId] = componentIndex;
            m_owners[componentIndex].push_back(entityId);

            invokeAndCleanup(m_addCallbacks, entityId, m_components[componentIndex].get());

            return component;
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

            invokeAndCleanup(m_removeCallbacks, entityId, m_components[componentIndex].get());

            // remove entity from owner list
            std::list<unsigned int> allOwners = m_owners[componentIndex];
            allOwners.remove(entityId);

            // remove component if there are no owners
            if (allOwners.size() == 0)
            {
                auto it = m_components.begin();
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

        ComponentType* getComponent(unsigned int entityId)
        {
            ensureEntity(entityId);

            if (m_sparse[entityId] == -1)
            {
                return nullptr;
            }

            return m_components[m_sparse[entityId]].get();
        }

        const std::vector<std::unique_ptr<ComponentType>>& getComponents()
        {
            return m_components;
        }

        const std::vector<std::list<unsigned int>>& getOwners() 
        {
            return m_owners;
        }

        const std::list<unsigned int>& getOwners(ComponentType* component)
        {
            // TODO: make this error safe, e.g. component isn't even stored
            for (unsigned int i{0}; i < m_owners.size(); ++i) {
                if (m_components[i].get() == component) {
                    return m_owners[i];
                }
            }
        }

        std::shared_ptr<component_table_callback> onAdded(component_table_callback && cb) {
            std::shared_ptr<component_table_callback> shared = std::make_shared<component_table_callback>(cb);
            m_addCallbacks.push_back(shared);
            return shared;
        }

        std::shared_ptr<component_table_callback> onRemove(component_table_callback && cb) {
            std::shared_ptr<component_table_callback> shared = std::make_shared<component_table_callback>(cb);
            m_removeCallbacks.push_back(shared);
            return shared;
        }

        std::shared_ptr<component_table_callback> onUpdate(unsigned int entityId, component_table_callback && cb) {
            if (m_sparse[entityId] > -1) {
                std::shared_ptr<component_table_callback> shared = std::make_shared<component_table_callback>(cb);
                m_updateCallbacks[m_sparse[entityId]].push_back(shared);
                return shared;
            }

            return std::shared_ptr<component_table_callback>{};
        }

        // called after one updated a coponent of an entity to signal the update to everyone who added a callback
        void updated(unsigned int entityId) {
            int componentIndex = m_sparse[entityId];
            if (componentIndex > -1) {
                invokeAndCleanup(m_updateCallbacks[componentIndex], entityId, m_components[componentIndex].get());
            }
        }
    };
}

#endif