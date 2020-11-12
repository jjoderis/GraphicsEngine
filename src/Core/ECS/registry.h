#ifndef CORE_ECS_REGISTRY
#define CORE_ECS_REGISTRY

#include "util.h"
#include "componentTable.h"
#include <vector>
#include <list>

namespace Engine{
    class Registry {
    private:
        std::vector<void*> m_componentLinks{};
        // ordered list of unused entity ids
        std::list<unsigned int> m_freeEntityIds{};
        // ordered list of used entities
        std::list<unsigned int> m_usedEntityIds{};
        unsigned int m_maxEntities = 0;

        std::vector<std::function<void(unsigned int)>> m_componentLinkCleaners{};

        template <typename ComponentType>
        ComponentTable<ComponentType>* ensureComponentTable() {
            if (type_index<ComponentType>::value() >= m_componentLinks.size()) 
            {
                m_componentLinks.resize(type_index<ComponentType>::value()+1, nullptr);
                m_componentLinkCleaners.resize(type_index<ComponentType>::value()+1, [](unsigned int foo) {});
            } 

            if (m_componentLinks[type_index<ComponentType>::value()] == nullptr)
            {
                m_componentLinks[type_index<ComponentType>::value()] = new ComponentTable<ComponentType>{m_maxEntities};
                m_componentLinkCleaners[type_index<ComponentType>::value()] = [&](unsigned int entity) {
                    this->removeComponent<ComponentType>(entity);
                };
            }

            return (ComponentTable<ComponentType>*)m_componentLinks[type_index<ComponentType>::value()];
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
            for (std::function<void(unsigned int)>& cleaner: m_componentLinkCleaners) {
                cleaner(index);
            }
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

            ComponentTable<ComponentType>* compTable = ensureComponentTable<ComponentType>();

            return compTable->addComponent(entityId, component);
        }

        template <typename ComponentType>
        bool hasComponent(unsigned int entityId)
        {
            if (entityId >= m_maxEntities)
            {
                throw "EntityId out of bounds\n";
            }

            ComponentTable<ComponentType>* compTable = ensureComponentTable<ComponentType>();

            return compTable->hasComponent(entityId);
        }

        template <typename ComponentType>
        ComponentType* getComponent(unsigned int entityId)
        {
            if (entityId >= m_maxEntities)
            {
                throw "EntityId out of bounds\n";
            }

            ComponentTable<ComponentType>* compTable = ensureComponentTable<ComponentType>();

            return compTable->getComponent(entityId);
        }

        template <typename ComponentType>
        void removeComponent(unsigned int entityId)
        {
            if (entityId >= m_maxEntities)
            {
                throw "EntityId out of bounds\n";
            }
            
            ComponentTable<ComponentType>* compTable = ensureComponentTable<ComponentType>();

            compTable->removeComponent(entityId);
        }

        template <typename ComponentType>
        const std::vector<std::unique_ptr<ComponentType>>& getComponents()
        {
            ComponentTable<ComponentType>* compTable = ensureComponentTable<ComponentType>();

            return compTable->getComponents();
        }

        // returns all owners for all components of a specific type
        template <typename ComponentType>
        const std::vector<std::list<unsigned int>>& getOwners()
        {
            ComponentTable<ComponentType>* compTable = ensureComponentTable<ComponentType>();

            return compTable->getOwners();
        }

        template <typename ComponentType>
        const std::list<unsigned int>& getOwners(ComponentType* component)
        {
            ComponentTable<ComponentType>* compTable = ensureComponentTable<ComponentType>();

            return compTable->getOwners(component);
        }

        template <typename ComponentType>
        std::shared_ptr<std::function<void(unsigned int, ComponentType*)>> onAdded(std::function<void(unsigned int, ComponentType*)> && cb)
        {
            ComponentTable<ComponentType>* compTable = ensureComponentTable<ComponentType>();

            return compTable->onAdded(std::move(cb));
        }

        template <typename ComponentType>
        std::shared_ptr<std::function<void(unsigned int, ComponentType*)>> onRemove(std::function<void(unsigned int, ComponentType*)> && cb)
        {
            ComponentTable<ComponentType>* compTable = ensureComponentTable<ComponentType>();

            return compTable->onRemove(std::move(cb));
        }

        template <typename ComponentType>
        std::shared_ptr<std::function<void(unsigned int, ComponentType*)>> onUpdate(unsigned int entityId, std::function<void(unsigned int, ComponentType*)> && cb)
        {
            ComponentTable<ComponentType>* compTable = ensureComponentTable<ComponentType>();

            return compTable->onUpdate(entityId, std::move(cb));
        }

        template <typename ComponentType>
        void updated(unsigned int entity) {
            ComponentTable<ComponentType>* compTable = ensureComponentTable<ComponentType>();

            compTable->updated(entity);
        }

        template <typename TypeA, typename TypeB>
        std::vector<std::pair<TypeA*, std::vector<TypeB*>>> getGroupedComponents()
        {
            const std::vector<std::unique_ptr<TypeA>>& aComponents = getComponents<TypeA>();
            const std::vector<std::list<unsigned int>>& aOwners = getOwners<TypeA>();

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