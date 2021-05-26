#include "hierarchy.h"

void Engine::HierarchyComponent::setParent(int parent) { m_parent = parent; }
void Engine::HierarchyComponent::unsetParent() { m_parent = -1; }
int Engine::HierarchyComponent::getParent() const { return m_parent; }

void Engine::HierarchyComponent::addChild(unsigned int child) { m_children.emplace_back(child); }
void Engine::HierarchyComponent::removeChild(unsigned int child)
{
    // find child index
    int index = m_children.size();
    for (int i = 0; i < m_children.size(); ++i)
    {
        if (m_children[i] == child)
        {
            index = i;
            break;
        }
    }

    // move up all following children and remove last entry
    for (int i = index + 1; i < m_children.size(); ++i)
    {
        m_children[i - 1] = m_children[i];
    }

    // if we found the child reduce size by 1
    if (index < m_children.size())
    {
        m_children.pop_back();
    }
}
bool Engine::HierarchyComponent::hasChild(unsigned int child) const
{
    for (int i = 0; i < m_children.size(); ++i)
    {
        if (m_children[i] == child)
        {
            return true;
        }
    }

    return false;
}
const std::vector<unsigned int> &Engine::HierarchyComponent::getChildren() { return m_children; }