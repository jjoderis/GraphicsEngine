#include <Core/ECS/componentTable.h>
#include <gtest/gtest.h>

TEST(ECS_COMPONENT_TABLE_TEST, instanciable) { Engine::ComponentTable<int> a{1}; }

TEST(ECS_COMPONENT_TABLE_TEST, addComponent)
{
    Engine::ComponentTable<int> a{4};

    std::shared_ptr<int> comp = std::make_shared<int>(5);

    std::shared_ptr<int> storedComp = a.addComponent(2, comp);

    std::vector<std::shared_ptr<int>> internalComponents{a.getComponents()};
    std::vector<std::list<unsigned int>> internalOwners{a.getOwners()};

    EXPECT_EQ(storedComp, comp);
    EXPECT_EQ(internalComponents.size(), 1);
    EXPECT_EQ(internalOwners.size(), 1);
    EXPECT_EQ(internalOwners[0].size(), 1);
    EXPECT_EQ(internalComponents[0], comp);
    EXPECT_EQ(internalOwners[0].front(), 2);

    std::shared_ptr<int> otherComp = std::make_shared<int>(10);
    std::shared_ptr<int> storedOtherComp = a.addComponent(1, otherComp);

    internalComponents = a.getComponents();
    internalOwners = a.getOwners();

    EXPECT_EQ(storedOtherComp, otherComp);
    EXPECT_EQ(internalComponents.size(), 2);
    EXPECT_EQ(internalOwners.size(), 2);
    EXPECT_EQ(internalOwners[0].size(), 1);
    EXPECT_EQ(internalOwners[1].size(), 1);
    EXPECT_EQ(internalComponents[0], comp);
    EXPECT_EQ(internalOwners[0].front(), 2);
    EXPECT_EQ(internalComponents[1], otherComp);
    EXPECT_EQ(internalOwners[1].front(), 1);

    std::shared_ptr<int> overWriteComponent = a.addComponent(1, comp);

    internalComponents = a.getComponents();
    internalOwners = a.getOwners();

    EXPECT_EQ(overWriteComponent, comp);
    EXPECT_EQ(internalComponents.size(), 1);
    EXPECT_EQ(internalOwners.size(), 1);
    EXPECT_EQ(internalOwners[0].size(), 2);
    EXPECT_EQ(internalComponents[0], comp);
    EXPECT_EQ(internalOwners[0].front(), 2);
    EXPECT_EQ(internalOwners[0].back(), 1);

    overWriteComponent = a.addComponent(1, comp);

    EXPECT_EQ(overWriteComponent, comp);
    EXPECT_EQ(internalComponents.size(), 1);
    EXPECT_EQ(internalOwners.size(), 1);
    EXPECT_EQ(internalOwners[0].size(), 2);
    EXPECT_EQ(internalComponents[0], comp);
    EXPECT_EQ(internalOwners[0].front(), 2);
    EXPECT_EQ(internalOwners[0].back(), 1);
}

TEST(ECS_COMPONENT_TABLE_TEST, hasComponent)
{
    Engine::ComponentTable<int> a{1};

    EXPECT_FALSE(a.hasComponent(0));

    std::shared_ptr<int> comp = std::make_shared<int>(5);
    a.addComponent(0, comp);

    EXPECT_TRUE(a.hasComponent(0));
}

TEST(ECS_COMPONENT_TABLE_TEST, getComponent)
{
    Engine::ComponentTable<int> a{1};

    std::shared_ptr<int> storedComp = a.getComponent(0);

    EXPECT_FALSE(storedComp);

    std::shared_ptr<int> comp = std::make_shared<int>(5);
    a.addComponent(0, comp);

    storedComp = a.getComponent(0);

    EXPECT_EQ(comp, storedComp);
}

TEST(ECS_COMPONENT_TABLE_TEST, getOwners_for_component)
{
    Engine::ComponentTable<int> a{3};

    std::shared_ptr<int> componentA = a.addComponent(0, std::make_shared<int>(2));
    std::shared_ptr<int> componentB = a.addComponent(1, std::make_shared<int>(4));
    a.addComponent(2, componentA);

    const std::list<unsigned int> ownersA = a.getOwners(componentA);
    const std::list<unsigned int> ownersB = a.getOwners(componentB);

    EXPECT_EQ(ownersA.size(), 2);
    EXPECT_EQ(ownersA.front(), 0);
    EXPECT_EQ(ownersA.back(), 2);

    EXPECT_EQ(ownersB.size(), 1);
    EXPECT_EQ(ownersB.back(), 1);
}

TEST(ECS_COMPONENT_TABLE_TEST, onAdded)
{
    Engine::ComponentTable<int> a{2};

    std::shared_ptr<int> componentA = std::make_shared<int>(0);
    std::shared_ptr<int> componentB = std::make_shared<int>(0);

    std::shared_ptr<int> addedToFirst{nullptr};

    std::shared_ptr<int> addedToSecond{nullptr};

    int invokeCounter{0};

    // The shared_ptr hold the callback alive; if it goes out of scope or isn't even stored the callback will be cleaned
    // up
    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<int>)>> cb1 = a.onAdded(
        [&](unsigned int entity, std::weak_ptr<int> component)
        {
            if (entity == 0)
            {
                addedToFirst = component.lock();
            }
            invokeCounter++;
        });

    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<int>)>> cb2 = a.onAdded(
        [&](unsigned int entity, std::weak_ptr<int> component)
        {
            if (entity == 1)
            {
                addedToSecond = component.lock();
            }
        });

    a.addComponent(1, componentA);
    a.addComponent(0, componentB);

    EXPECT_EQ(addedToFirst, componentB);
    EXPECT_EQ(addedToSecond, componentA);
    EXPECT_EQ(invokeCounter, 2);

    // check cleanup of callback on deletion of shared_ptr
    cb1.reset();

    a.addComponent(1, componentA);
    EXPECT_EQ(invokeCounter, 2);
}

TEST(ECS_COMPONENT_TABLE_TEST, onRemove)
{
    Engine::ComponentTable<int> a{2};

    std::shared_ptr<int> componentA = std::make_shared<int>(0);
    std::shared_ptr<int> componentB = std::make_shared<int>(0);

    std::shared_ptr<int> removedFirst{nullptr};

    std::shared_ptr<int> removedSecond{nullptr};

    int counter{0};

    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<int>)>> cb1 = a.onRemove(
        [&](unsigned int entity, std::weak_ptr<int> component)
        {
            if (entity++ == 0)
            {
                removedFirst = component.lock();
            }
        });

    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<int>)>> cb2 = a.onRemove(
        [&](unsigned int entity, std::weak_ptr<int> component)
        {
            if (counter++ == 1)
            {
                removedSecond = component.lock();
            }
        });

    a.addComponent(1, componentA);
    a.addComponent(0, componentB);

    a.removeComponent(0);
    a.removeComponent(1);

    EXPECT_EQ(removedFirst, componentB);
    EXPECT_EQ(removedSecond, componentA);
}

TEST(ECS_COMPONENT_TABLE_TEST, onUpdate)
{
    Engine::ComponentTable<int> a{2};

    std::shared_ptr<int> componentA = a.addComponent(0, std::make_shared<int>(0));
    std::shared_ptr<int> componentB = a.addComponent(1, std::make_shared<int>(0));

    int counter{0};

    std::shared_ptr<std::function<void(unsigned int, std::weak_ptr<int>)>> cb =
        a.onUpdate(1, [&](unsigned int entity, std::weak_ptr<int> component) { counter++; });

    a.updated(1);
    a.updated(1);
    a.updated(0);

    EXPECT_EQ(counter, 2);
}