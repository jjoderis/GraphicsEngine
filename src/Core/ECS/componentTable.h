#ifndef CORE_ECS_COMPONENTTABLE
#define CORE_ECS_COMPONENTTABLE

#include <vector>
#include <list>
#include <memory>

namespace Core {
    template<typename ComponentType>
    struct ComponentTable {
    private:
        std::vector<int> m_sparse;
        std::vector<std::unique_ptr<ComponentType>> m_components{};
        std::vector<std::list<unsigned int>> m_owners{};

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
            return m_components.size() - 1;
        }

        void ensureEntity(unsigned int entityId)
        {
            if (entityId >= m_sparse.size())
            {
                m_sparse.resize(entityId+1, -1);
            }
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

            // remove entity from owner list
            std::list<unsigned int> allOwners = m_owners[componentIndex];
            allOwners.remove(entityId);

            // remove component if there are no owners
            if (allOwners.size() == 0)
            {
                auto it = m_components.begin();
                m_components.erase(it + componentIndex);
                m_owners.erase(m_owners.begin() + componentIndex);
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

        std::vector<std::unique_ptr<ComponentType>>& getComponents()
        {
            return m_components;
        }

        std::vector<std::list<unsigned int>>& getOwners() 
        {
            return m_owners;
        }
    };
}

#endif