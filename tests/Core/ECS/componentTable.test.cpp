#include <Core/ECS/componentTable.h>
#include <gtest/gtest.h>

TEST(ECS_COMPONENT_TABLE_TEST, instanciable)
{
    Core::ComponentTable<int> a{ 1 };
}

TEST(ECS_COMPONENT_TABLE_TEST, addComponent)
{
    Core::ComponentTable<int> a{ 4 };
    const std::vector<std::unique_ptr<int>>& internalComponents{ a.getComponents() };
    const std::vector<std::list<unsigned int>>& internalOwners{ a.getOwners() };

    int* comp = new int{5};

    int* storedComp = a.addComponent(2, comp);

    EXPECT_EQ(storedComp, comp);
    EXPECT_EQ(internalComponents.size(), 1);
    EXPECT_EQ(internalOwners.size(), 1);
    EXPECT_EQ(internalOwners[0].size(), 1);
    EXPECT_EQ(internalComponents[0].get(), comp);
    EXPECT_EQ(internalOwners[0].front(), 2);

    int* otherComp = new int{10};
    int* storedOtherComp = a.addComponent(1, otherComp);

    EXPECT_EQ(storedOtherComp, otherComp);
    EXPECT_EQ(internalComponents.size(), 2);
    EXPECT_EQ(internalOwners.size(), 2);
    EXPECT_EQ(internalOwners[0].size(), 1);
    EXPECT_EQ(internalOwners[1].size(), 1);
    EXPECT_EQ(internalComponents[0].get(), comp);
    EXPECT_EQ(internalOwners[0].front(), 2);
    EXPECT_EQ(internalComponents[1].get(), otherComp);
    EXPECT_EQ(internalOwners[1].front(), 1);

    int* overWriteComponent = a.addComponent(1, comp);

    EXPECT_EQ(overWriteComponent, comp);
    EXPECT_EQ(internalComponents.size(), 1);
    EXPECT_EQ(internalOwners.size(), 1);
    EXPECT_EQ(internalOwners[0].size(), 2);
    EXPECT_EQ(internalComponents[0].get(), comp);
    EXPECT_EQ(internalOwners[0].front(), 2);
    EXPECT_EQ(internalOwners[0].back(), 1);

    overWriteComponent = a.addComponent(1, comp);

    EXPECT_EQ(overWriteComponent, comp);
    EXPECT_EQ(internalComponents.size(), 1);
    EXPECT_EQ(internalOwners.size(), 1);
    EXPECT_EQ(internalOwners[0].size(), 2);
    EXPECT_EQ(internalComponents[0].get(), comp);
    EXPECT_EQ(internalOwners[0].front(), 2);
    EXPECT_EQ(internalOwners[0].back(), 1);
}

TEST(ECS_COMPONENT_TABLE_TEST, hasComponent)
{
    Core::ComponentTable<int> a{ 1 };

    EXPECT_FALSE(a.hasComponent(0));

    int* comp = new int{5};
    a.addComponent(0, comp);

    EXPECT_TRUE(a.hasComponent(0));
}

TEST(ECS_COMPONENT_TABLE_TEST, getComponent)
{
    Core::ComponentTable<int> a{ 1 };

    int* storedComp = a.getComponent(0);

    EXPECT_EQ(nullptr, storedComp);

    int* comp = new int{5};
    a.addComponent(0, comp);

    storedComp = a.getComponent(0);

    EXPECT_EQ(comp, storedComp);
}