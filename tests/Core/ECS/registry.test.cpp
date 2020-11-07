#include <Core/ECS/registry.h>
#include <gtest/gtest.h>

#include <string>
#include <exception>

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

TEST(ECS_REGISTRY_TEST, addComponent)
{
    // you are able to add any type of Component to an entity
    Core::Registry a{};

    int entity = a.addEntity();

    std::string* newComp = a.addComponent<std::string>(entity, new std::string{"Hello World"});

    EXPECT_STREQ(newComp->c_str(), "Hello World");

    // adding a component to an entity that already owns the component changes nothing
    std::string* sameComp = a.addComponent<std::string>(entity, newComp);

    EXPECT_EQ(newComp, sameComp);

    // entities can share a component
    int otherEntity = a.addEntity();

    std::string* otherEntityComponent = a.addComponent<std::string>(otherEntity, newComp);

    EXPECT_EQ(otherEntityComponent, newComp);

    // throws on invalid entity id
    EXPECT_ANY_THROW(a.addComponent(3, newComp));
}

TEST(ECS_REGISTRY_TEST, hasComponent)
{
    Core::Registry a{};

    int entityA = a.addEntity();
    int entityB = a.addEntity();

    // returns true if entity has a component of the given type
    a.addComponent(entityA, new int{1});
    EXPECT_TRUE(a.hasComponent<int>(entityA));

    // returns false if there is no list for the given type
    EXPECT_FALSE(a.hasComponent<double>(entityA));

    // returns false if an entity doesn't have a component of the given type
    EXPECT_FALSE(a.hasComponent<int>(entityB));
}

TEST(ECS_REGISTRY_TEST, getComponent)
{
    Core::Registry a{};

    int entity = a.addEntity();

    // returns the component if the entity has one
    std::string* stringComp = new std::string{""};
    a.addComponent<std::string>(entity, stringComp);
    EXPECT_EQ(a.getComponent<std::string>(entity), stringComp);

    // returns nullptr if there is no list for the component type
    EXPECT_EQ(a.getComponent<float>(entity), nullptr);

    // returns nullptr if the entity doesn't own a component of this type
    int newEntity = a.addEntity();
    EXPECT_EQ(a.getComponent<std::string>(newEntity), nullptr);
}

TEST(ECS_REGISTRY_TEST, removeComponent) 
{
    Core::Registry a{};

    int entity = a.addEntity();

    std::string* stringComp = new std::string{""};
    a.addComponent<std::string>(entity, stringComp);
    EXPECT_EQ(a.getComponent<std::string>(entity), stringComp);

    // removes a component from an entity if the entity has one
    a.removeComponent<std::string>(entity);
    EXPECT_EQ(a.getComponent<std::string>(entity), nullptr);
}

TEST(ECS_REGISTRY_TEST, getComponents)
{
    Core::Registry a{};

    int entity = a.addEntity();
    int entityB = a.addEntity();

    std::string* stringCompA = new std::string{""};
    std::string* stringCompB = new std::string{""};

    a.addComponent(entity, stringCompA);
    a.addComponent(entityB, stringCompB);

    std::vector<std::unique_ptr<std::string>>& stringComps = a.getComponents<std::string>();

    EXPECT_EQ(stringComps.size(), 2);
    EXPECT_EQ(stringComps[0].get(), stringCompA);
    EXPECT_EQ(stringComps[1].get(), stringCompB);
}

TEST(ECS_REGISTRY_TEST, getGroupedComponents)
{
    Core::Registry a{};

    int entityA = a.addEntity();
    int entityB = a.addEntity();
    int entityC = a.addEntity();

    std::string* stringComp1 = new std::string{"Heyllo"};
    std::string* stringComp2 = new std::string{""};

    a.addComponent(entityA, stringComp1);
    a.addComponent(entityB, stringComp2);
    a.addComponent(entityC, stringComp2);

    int* int1 = new int{1};
    int* int2 = new int{2};

    a.addComponent(entityB, int1);
    a.addComponent(entityC, int2);

    std::vector<std::pair<std::string*, std::vector<int*>>> groupStrInt = a.getGroupedComponents<std::string, int>();

    EXPECT_EQ(groupStrInt.size(), 2);

    EXPECT_EQ(groupStrInt[0].second.size(), 0);

    EXPECT_EQ(groupStrInt[1].second.size(), 2);
    EXPECT_EQ(groupStrInt[1].second[0], int1);
    EXPECT_EQ(groupStrInt[1].second[1], int2);
}