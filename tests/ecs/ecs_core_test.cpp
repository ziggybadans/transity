#include <gtest/gtest.h>
#include <SFML/Graphics.hpp>

#include <ostream>
#include <memory>
#include <vector>
#include <string>

#include "ecs/ECSCore.hpp"
#include "ecs/ISystem.h"

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

class MockUpdateSystem : public transity::ecs::IUpdateSystem {
public:
    bool updated = false;
    float lastDeltaTime = -1.0f;

    void update(entt::registry& registry, float deltaTime) override {
        updated = true;
        lastDeltaTime = deltaTime;
    }
};

class MockOrderedUpdateSystem : public transity::ecs::IUpdateSystem {
public:
    std::vector<std::string>& executionOrderRef;
    std::string id;

    MockOrderedUpdateSystem(std::vector<std::string>& orderList, const std::string identifier)
        : executionOrderRef(orderList), id(identifier) {}
    
    void update(entt::registry& registry, float deltaTime) override {
        executionOrderRef.push_back(id);
    }
};

class MockRenderSystem : public transity::ecs::IRenderSystem {
public:
    bool rendered = false;
    sf::RenderTarget* lastRenderTarget = nullptr;

    void render(entt::registry& registry, sf::RenderTarget& renderTarget) override {
        rendered = true;
        lastRenderTarget = &renderTarget;
    }
};

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

TEST(ECSCoreTest, ViewSingleComponent) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();
    struct PositionComponent { float x = 0.0f; float y = 0.0f; };
    struct VelocityComponent { float dx = 0.0f; float dy = 0.0f; };
    entt::entity entity1 = ecsCore.createEntity();
    entt::entity entity2 = ecsCore.createEntity();
    entt::entity entity3 = ecsCore.createEntity();

    ecsCore.addComponent<PositionComponent>(entity1, 1.0f, 1.0f);
    ecsCore.addComponent<PositionComponent>(entity3, 3.0f, 3.0f);
    ecsCore.addComponent<VelocityComponent>(entity3);

    auto positionView = ecsCore.getView<PositionComponent>();

    size_t count = 0;
    bool foundEntity1 = false;
    bool foundEntity3 = false;
    for(auto entity : positionView) {
        count++;
        if(entity == entity1) {
            foundEntity1 = true;
        }
        if(entity == entity3) {
            foundEntity3 = true;
        }

        auto& pos = positionView.get<PositionComponent>(entity);
        ASSERT_NE(pos.x, 0.0f);
    }

    ASSERT_EQ(count, 2);
    ASSERT_TRUE(foundEntity1);
    ASSERT_TRUE(foundEntity3);
}

TEST(ECSCoreTest, ViewMultiComponent) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();
    struct PositionComponent { float x = 0.0f; float y = 0.0f; };
    struct VelocityComponent { float dx = 0.0f; float dy = 0.0f; };
    entt::entity entity1 = ecsCore.createEntity();
    entt::entity entity2 = ecsCore.createEntity();
    entt::entity entity3 = ecsCore.createEntity();
    entt::entity entity4 = ecsCore.createEntity();

    ecsCore.addComponent<PositionComponent>(entity1, 1.0f, 1.0f);
    ecsCore.addComponent<VelocityComponent>(entity2, 2.0f, 2.0f);
    ecsCore.addComponent<PositionComponent>(entity3, 3.0f, 3.0f);
    ecsCore.addComponent<VelocityComponent>(entity3, 3.0f, 3.0f);

    auto multiView = ecsCore.getView<PositionComponent, VelocityComponent>();

    size_t count = 0;
    bool foundEntity3 = false;
    for(auto entity : multiView) {
        count++;
        if(entity == entity3) {
            foundEntity3 = true;
        }

        auto& pos = multiView.get<PositionComponent>(entity);
        auto& vel = multiView.get<VelocityComponent>(entity);
    }

    ASSERT_EQ(count, 1);
    ASSERT_TRUE(foundEntity3);
}

TEST(ECSCoreTest, ViewEmpty) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();
    struct PositionComponent { float x = 0.0f; float y = 0.0f; };
    struct VelocityComponent { float dx = 0.0f; float dy = 0.0f; };
    struct RenderableComponent { int layer = 0; };

    entt::entity entity1 = ecsCore.createEntity();
    entt::entity entity2 = ecsCore.createEntity();
    ecsCore.addComponent<PositionComponent>(entity1);
    ecsCore.addComponent<VelocityComponent>(entity2);

    auto renderableView = ecsCore.getView<RenderableComponent>();
    size_t count = 0;
    for(auto entity : renderableView) {
        count++;
    }
    ASSERT_EQ(count, 0);
    ASSERT_TRUE(renderableView.empty());
}

TEST(ECSCoreTest, SystemRegistration) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();

    auto mockUpdate = std::make_unique<MockUpdateSystem>();
    auto mockRender = std::make_unique<MockRenderSystem>();

    MockUpdateSystem* mockUpdatePtr = mockUpdate.get();
    MockRenderSystem* mockRenderPtr = mockRender.get();

    ecsCore.registerUpdateSystem(std::move(mockUpdate));
    ecsCore.registerRenderSystem(std::move(mockRender));

    SUCCEED();
}

TEST(ECSCoreTest, UpdateSystemsExecution) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();

    auto mockUpdate = std::make_unique<MockUpdateSystem>();
    MockUpdateSystem* mockUpdatePtr = mockUpdate.get();

    ecsCore.registerUpdateSystem(std::move(mockUpdate));

    ASSERT_FALSE(mockUpdatePtr->updated);
    ASSERT_EQ(mockUpdatePtr->lastDeltaTime, -1.0f);

    float testDeltaTime = 0.16f;
    ecsCore.updateSystems(testDeltaTime);

    ASSERT_TRUE(mockUpdatePtr->updated);
    ASSERT_FLOAT_EQ(mockUpdatePtr->lastDeltaTime, testDeltaTime);
}

TEST(ECSCoreTest, RenderSystemsExecution) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();

    auto mockRender = std::make_unique<MockRenderSystem>();
    MockRenderSystem* mockRenderPtr = mockRender.get();

    ecsCore.registerRenderSystem(std::move(mockRender));

    ASSERT_FALSE(mockRenderPtr->rendered);
    ASSERT_EQ(mockRenderPtr->lastRenderTarget, nullptr);

    sf::RenderTexture dummyTarget;
    ecsCore.renderSystems(dummyTarget);

    ASSERT_TRUE(mockRenderPtr->rendered);
    ASSERT_EQ(mockRenderPtr->lastRenderTarget, &dummyTarget);
}

TEST(ECSCoreTest, ShutdownRegistryClear) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();
    struct PositionComponent { float x = 0.0f; float y = 0.0f; };

    entt::entity entity = ecsCore.createEntity();
    ecsCore.addComponent<PositionComponent>(entity, 1.0f, 2.0f);

    ecsCore.registerUpdateSystem(std::make_unique<MockUpdateSystem>());

    ASSERT_TRUE(ecsCore.hasEntity(entity));
    ASSERT_TRUE(ecsCore.hasComponent<PositionComponent>(entity));
    ASSERT_GT(ecsCore.getEntityCount(), 0);

    ecsCore.shutdown();

    ASSERT_FALSE(ecsCore.hasEntity(entity));
    ASSERT_EQ(ecsCore.getEntityCount(), 0);

    ecsCore.initialize();
    entt::entity newEntity = ecsCore.createEntity();
    ASSERT_NE(newEntity, entt::null);
}

TEST(ECSCoreTest, UpdateSystemsOrder) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();

    std::vector<std::string> executionOrder;

    auto system1 = std::make_unique<MockOrderedUpdateSystem>(executionOrder, "System1");
    auto system2 = std::make_unique<MockOrderedUpdateSystem>(executionOrder, "System2");

    ecsCore.registerUpdateSystem(std::move(system1));
    ecsCore.registerUpdateSystem(std::move(system2));

    ecsCore.updateSystems(0.1f);

    ASSERT_EQ(executionOrder.size(), 2);
    ASSERT_EQ(executionOrder[0], "System1");
    ASSERT_EQ(executionOrder[1], "System2");
}

TEST(ECSCoreTest, GetEntityCount) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();

    ASSERT_EQ(ecsCore.getEntityCount(), 0);

    entt::entity entity1 = ecsCore.createEntity();
    ASSERT_EQ(ecsCore.getEntityCount(), 1);

    entt::entity entity2 = ecsCore.createEntity();
    ASSERT_EQ(ecsCore.getEntityCount(), 2);

    entt::entity entity3 = ecsCore.createEntity();
    ASSERT_EQ(ecsCore.getEntityCount(), 3);

    ecsCore.destroyEntity(entity2);
    ASSERT_EQ(ecsCore.getEntityCount(), 2);

    ecsCore.destroyEntity(entity1);
    ASSERT_EQ(ecsCore.getEntityCount(), 1);

    ecsCore.destroyEntity(entity3);
    ASSERT_EQ(ecsCore.getEntityCount(), 0);
}

TEST(ECSCoreTest, HasEntityNull) {
    transity::ecs::ECSCore ecsCore;
    ecsCore.initialize();

    ASSERT_FALSE(ecsCore.hasEntity(entt::null));
}