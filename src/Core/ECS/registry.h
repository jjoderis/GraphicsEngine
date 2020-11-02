#ifndef CORE_ECS_REGISTRY
#define CORE_ECS_REGISTRY

#include "util.h"
#include <vector>
#include <list>

namespace Core{
    class Registry {
    private:
        std::vector<void*> m_componentLinks{};
        std::list<unsigned int> m_freeEntityIds{};
        unsigned int m_maxEntities = 0;
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

            return freeIndex;
        }

        void removeEntity(unsigned int index)
        {
            m_freeEntityIds.remove(index);
            m_freeEntityIds.push_front(index);
        }

        template <typename ComponentType>
        ComponentType& addComponent(unsigned int entityId, ComponentType* component) {
            if (type_index<ComponentType>::value() >= m_componentLinks.size()) {
                m_componentLinks.reserve(type_index<ComponentType>::value()+1);

            }
        }
    };
}

#endif