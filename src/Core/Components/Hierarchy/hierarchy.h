#ifndef CORE_COMPONENTS_HIERARCHY
#define CORE_COMPONENTS_HIERARCHY

#include <vector>

namespace Engine
{
class Registry;

class HierarchyComponent
{
public:
    // sets the id of new entity as parent
    void setParent(int parent);
    // sets to no parent
    void unsetParent();
    int getParent() const;

    // adds a child
    void addChild(unsigned int child);
    // removes a child
    void removeChild(unsigned int child);
    bool hasChild(unsigned int child) const;
    const std::vector<unsigned int> &getChildren();

    void getDecendants(std::vector<unsigned int> &decendants, Engine::Registry &registry);

private:
    int m_parent{-1};
    std::vector<unsigned int> m_children;
};

}; // namespace Engine

#endif