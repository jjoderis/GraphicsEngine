#include <Core/ECS/registry.h>
#include <gtest/gtest.h>

TEST(ECS_REGISTRY_TEST, instanciable)
{
    Core::Registry a{};
}

TEST(ECS_REGISTRY_TEST, addEntity)
{
    Core::Registry a{};

    unsigned int entityId1 = a.addEntity();
    unsigned int entityId2 = a.addEntity();

    EXPECT_EQ(entityId1, 0);
    EXPECT_EQ(entityId2, 1);
}

TEST(ECS_REGISTRY_TEST, removeEntity)
{
    Core::Registry a{};

    a.addEntity();
    a.addEntity();
    a.addEntity();
    a.addEntity();
    a.removeEntity(2);
    unsigned int newEnt = a.addEntity();

    EXPECT_EQ(newEnt, 2);
}
