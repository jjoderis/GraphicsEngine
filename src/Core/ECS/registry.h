#ifndef CORE_ECS_REGISTRY
#define CORE_ECS_REGISTRY

#include "util.h"
#include "componentTable.h"
#include <vector>
#include <list>

namespace Core{
    class Registry {
    private:
        std::vector<void*> m_componentLinks{};
        // ordered list of unused entity ids
        std::list<unsigned int> m_freeEntityIds{};
        // ordered list of used entities
        std::list<unsigned int> m_usedEntityIds{};
        unsigned int m_maxEntities = 0;

        template <typename ComponentType>
        std::vector<std::list<unsigned int>>& getOwners()
        {
           if (type_index<ComponentType>::value() >= m_componentLinks.size()) 
            {
                m_componentLinks.resize(type_index<ComponentType>::value()+1, nullptr);
            } 

            if (m_componentLinks[type_index<ComponentType>::value()] == nullptr)
            {
                m_componentLinks[type_index<ComponentType>::value()] = new ComponentTable<ComponentType>{m_maxEntities};
            }

            ComponentTable<ComponentType>* compTable =
                (ComponentTable<ComponentType>*)m_componentLinks[type_index<ComponentType>::value()];

            return compTable->getOwners();
        }
    public:
        Registry() {}
        Registry(const Registry &other) = delete;

        unsigned int addEntity()
        {
            unsigned int freeIndex;
            if (!m_freeEntityIds.empty()) {
                freeIndex = m_freeEntityIds.front();
                m_freeEntityIds.pop_front();
            } else {
                freeIndex = m_maxEntities++;
            }

            m_usedEntityIds.push_back(freeIndex);

            return freeIndex;
        }

        // returns a list of all used entity indices
        const std::list<unsigned int>& getEntities() {
            return m_usedEntityIds;
        }

        void removeEntity(unsigned int index)
        {
            m_freeEntityIds.remove(index);
            m_usedEntityIds.remove(index);
            m_freeEntityIds.push_front(index);
        }

        template <typename ComponentType>
        ComponentType* addComponent(unsigned int entityId, ComponentType* component) {
            if (entityId >= m_maxEntities)
            {
                throw "EntityId out of bounds\n";
            }

            if (type_index<ComponentType>::value() >= m_componentLinks.size()) 
            {
                m_componentLinks.resize(type_index<ComponentType>::value()+1, nullptr);
            } 

            if (m_componentLinks[type_index<ComponentType>::value()] == nullptr)
            {
                m_componentLinks[type_index<ComponentType>::value()] = new ComponentTable<ComponentType>{m_maxEntities};
            }

            ComponentTable<ComponentType>* compTable =
                (ComponentTable<ComponentType>*)m_componentLinks[type_index<ComponentType>::value()];

            return compTable->addComponent(entityId, component);
        }

        template <typename ComponentType>
        bool hasComponent(unsigned int entityId)
        {
            if (entityId >= m_maxEntities)
            {
                throw "EntityId out of bounds\n";
            }

            if (type_index<ComponentType>::value() >= m_componentLinks.size() || m_componentLinks[type_index<ComponentType>::value()] == nullptr) 
            {
                return false; 
            }

            ComponentTable<ComponentType>* compTable =
                (ComponentTable<ComponentType>*)m_componentLinks[type_index<ComponentType>::value()];

            return compTable->hasComponent(entityId);
        }

        template <typename ComponentType>
        ComponentType* getComponent(unsigned int entityId)
        {
            if (entityId >= m_maxEntities)
            {
                throw "EntityId out of bounds\n";
            }

            if (type_index<ComponentType>::value() >= m_componentLinks.size() || m_componentLinks[type_index<ComponentType>::value()] == nullptr)
            {
                return nullptr; 
            }

            ComponentTable<ComponentType>* compTable =
                (ComponentTable<ComponentType>*)m_componentLinks[type_index<ComponentType>::value()];

            return compTable->getComponent(entityId);
        }

        template <typename ComponentType>
        void removeComponent(unsigned int entityId)
        {
            if (entityId >= m_maxEntities)
            {
                throw "EntityId out of bounds\n";
            }
            
            if (type_index<ComponentType>::value() >= m_componentLinks.size() || m_componentLinks[type_index<ComponentType>::value()] == nullptr) 
            {
                return; 
            }

            ComponentTable<ComponentType>* compTable =
                (ComponentTable<ComponentType>*)m_componentLinks[type_index<ComponentType>::value()];

            compTable->removeComponent(entityId);
        }

        template <typename ComponentType>
        std::vector<std::unique_ptr<ComponentType>>& getComponents()
        {
           if (type_index<ComponentType>::value() >= m_componentLinks.size()) 
            {
                m_componentLinks.resize(type_index<ComponentType>::value()+1, nullptr);
            } 

            if (m_componentLinks[type_index<ComponentType>::value()] == nullptr)
            {
                m_componentLinks[type_index<ComponentType>::value()] = new ComponentTable<ComponentType>{m_maxEntities};
            }

            ComponentTable<ComponentType>* compTable =
                (ComponentTable<ComponentType>*)m_componentLinks[type_index<ComponentType>::value()];

            return compTable->getComponents();
        }

        template <typename TypeA, typename TypeB>
        std::vector<std::pair<TypeA*, std::vector<TypeB*>>> getGroupedComponents()
        {
            std::vector<std::unique_ptr<TypeA>>& aComponents = getComponents<TypeA>();
            std::vector<std::list<unsigned int>>& aOwners = getOwners<TypeA>();

            std::vector<std::pair<TypeA*, std::vector<TypeB*>>> out{};
            out.reserve(aComponents.size());

            for(unsigned int i = 0; i < aComponents.size(); ++i)
            {
                TypeA* component = aComponents[i].get();
                std::vector<TypeB*> associated{};
                associated.reserve(aOwners[i].size());

                for(unsigned int owner : aOwners[i])
                {
                    TypeB* otherComponent = getComponent<TypeB>(owner);
                    if (otherComponent)
                    {
                        associated.push_back(otherComponent);
                    }
                }

                out.emplace_back(component, associated);
            }

            return out;
        }
    };
}

#endif