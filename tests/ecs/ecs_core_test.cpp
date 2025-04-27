#include <gtest/gtest.h>
#include "ecs/ECSCore.hpp"
#include <ostream>

namespace entt {
    inline std::ostream& operator<<(std::ostream& os, const entt::entity entity) {
        // Print the underlying integer representation
        return os << static_cast<std::underlying_type_t<entt::entity>>(entity);
    }
    
    // Printer for entt::null_t
    inline std::ostream& operator<<(std::ostream& os, const entt::null_t) {
        return os << "entt::null";
    }
}

TEST(ECSCoreTest, RegistryCreated) {
    ASSERT_NO_THROW({
        transity::ecs::ECSCore ecsCore;
        ecsCore.initialize();
    });
}

TEST(ECSCoreTest, EntityCreation) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();

    entt::entity entity1 = ecsCore.createEntity();
    ASSERT_NE(entity1, entt::null);
    entt::entity entity2 = ecsCore.createEntity();
    ASSERT_NE(entity2, entt::null);

    ASSERT_NE(entity1, entity2);

    ASSERT_TRUE(ecsCore.hasEntity(entity1));
    ASSERT_TRUE(ecsCore.hasEntity(entity2));
}

TEST(ECSCoreTest, EntityDestruction) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();

    entt::entity entity = ecsCore.createEntity();
    ASSERT_NE(entity, entt::null);
    ASSERT_TRUE(ecsCore.hasEntity(entity));

    ecsCore.destroyEntity(entity);

    ASSERT_FALSE(ecsCore.hasEntity(entity));
}

TEST(ECSCoreTest, EntityDestructionRemovesComponents) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();

    struct TestComponent {
        int value;
    };

    entt::entity entity = ecsCore.createEntity();
    ASSERT_NE(entity, entt::null);
    ASSERT_TRUE(ecsCore.hasEntity(entity));

    ecsCore.addComponent<TestComponent>(entity, 42);
    ASSERT_TRUE(ecsCore.hasComponent<TestComponent>(entity));

    ecsCore.destroyEntity(entity);
    ASSERT_FALSE(ecsCore.hasEntity(entity));
    ASSERT_FALSE(ecsCore.hasComponent<TestComponent>(entity));
}

TEST(ECSCoreTest, ComponentAddition) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();
    struct PositionComponent { float x = 0.0f; float y = 0.0f; };

    entt::entity entity = ecsCore.createEntity();
    ASSERT_NE(entity, entt::null);
    ASSERT_TRUE(ecsCore.hasEntity(entity));

    ecsCore.addComponent<PositionComponent>(entity, 1.0f, 2.0f);
    ASSERT_TRUE(ecsCore.hasComponent<PositionComponent>(entity));
}

TEST(ECSCoreTest, ComponentAdditionReplace) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();
    struct PositionComponent { float x = 0.0f; float y = 0.0f; };

    entt::entity entity = ecsCore.createEntity();
    ASSERT_NE(entity, entt::null);
    ASSERT_TRUE(ecsCore.hasEntity(entity));

    ecsCore.addComponent<PositionComponent>(entity, 1.0f, 2.0f);
    ASSERT_TRUE(ecsCore.hasComponent<PositionComponent>(entity));
    ASSERT_EQ(ecsCore.getComponent<PositionComponent>(entity).x, 1.0f);
    ASSERT_EQ(ecsCore.getComponent<PositionComponent>(entity).y, 2.0f);

    ecsCore.addComponent<PositionComponent>(entity, 3.0f, 4.0f);
    ASSERT_TRUE(ecsCore.hasComponent<PositionComponent>(entity));
    ASSERT_EQ(ecsCore.getComponent<PositionComponent>(entity).x, 3.0f);
    ASSERT_EQ(ecsCore.getComponent<PositionComponent>(entity).y, 4.0f);
}

TEST(ECSCoreTest, ComponentCheckExists) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();
    struct PositionComponent { float x = 0.0f; float y = 0.0f; };

    entt::entity entity = ecsCore.createEntity();
    ASSERT_NE(entity, entt::null);
    ASSERT_TRUE(ecsCore.hasEntity(entity));

    ecsCore.addComponent<PositionComponent>(entity);
    ASSERT_TRUE(ecsCore.hasComponent<PositionComponent>(entity));
}

TEST(ECSCoreTest, ComponentCheckNotExists) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();
    struct PositionComponent { float x = 0.0f; float y = 0.0f; };

    entt::entity entity = ecsCore.createEntity();
    ASSERT_NE(entity, entt::null);
    ASSERT_TRUE(ecsCore.hasEntity(entity));

    ASSERT_FALSE(ecsCore.hasComponent<PositionComponent>(entity));
}

TEST(ECSCoreTest, ComponentRetrievalExists) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();
    struct PositionComponent { float x = 0.0f; float y = 0.0f; };

    entt::entity entity = ecsCore.createEntity();
    ASSERT_NE(entity, entt::null);
    ASSERT_TRUE(ecsCore.hasEntity(entity));
    ecsCore.addComponent<PositionComponent>(entity, 1.0f, 2.0f);
    ASSERT_TRUE(ecsCore.hasComponent<PositionComponent>(entity));

    PositionComponent& pos = ecsCore.getComponent<PositionComponent>(entity);
    ASSERT_EQ(pos.x, 1.0f);
    ASSERT_EQ(pos.y, 2.0f);

    pos.x = 50.0f;
    ASSERT_FLOAT_EQ(ecsCore.getComponent<PositionComponent>(entity).x, 50.0f);

    const transity::ecs::ECSCore& constEcsCore = ecsCore;
    const PositionComponent& constPos = constEcsCore.getComponent<PositionComponent>(entity);
    ASSERT_EQ(constPos.x, 50.0f);
    ASSERT_EQ(constPos.y, 2.0f);
}

TEST(ECSCoreTest, ComponentRetrievalNotExists) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();
    struct PositionComponent { float x = 0.0f; float y = 0.0f; };

    entt::entity entity = ecsCore.createEntity();
    ASSERT_NE(entity, entt::null);
    ASSERT_TRUE(ecsCore.hasEntity(entity));

    ASSERT_THROW(ecsCore.getComponent<PositionComponent>(entity), std::exception);

    const transity::ecs::ECSCore& constEcsCore = ecsCore;
    ASSERT_THROW(constEcsCore.getComponent<PositionComponent>(entity), std::exception);
}

TEST(ECSCoreTest, ComponentRemoval) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();
    struct PositionComponent { float x = 0.0f; float y = 0.0f; };
    entt::entity entity = ecsCore.createEntity();
    ASSERT_NE(entity, entt::null);
    ASSERT_TRUE(ecsCore.hasEntity(entity));

    ecsCore.addComponent<PositionComponent>(entity, 1.0f, 2.0f);
    ASSERT_TRUE(ecsCore.hasComponent<PositionComponent>(entity));

    ecsCore.removeComponent<PositionComponent>(entity);
    ASSERT_FALSE(ecsCore.hasComponent<PositionComponent>(entity));
    ASSERT_NO_THROW(ecsCore.removeComponent<PositionComponent>(entity));
}