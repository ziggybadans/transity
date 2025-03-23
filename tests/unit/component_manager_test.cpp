#include <catch2/catch_test_macros.hpp>
#include "transity/ecs/component_manager.hpp"

using namespace transity::ecs;

// Test component struct
struct TestComponent {
    int value = 0;
};

struct PositionComponent {
    float x = 0.0f;
    float y = 0.0f;
};

TEST_CASE("ComponentManager basic operations", "[ecs]") {
    ComponentManager manager;

    SECTION("Add and retrieve component") {
        const size_t entityId = 1;
        auto& comp = manager.addComponent<TestComponent>(entityId);
        comp.value = 42;

        REQUIRE(manager.hasComponent<TestComponent>(entityId));
        auto* retrieved = manager.getComponent<TestComponent>(entityId);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->value == 42);
    }

    SECTION("Remove component") {
        const size_t entityId = 1;
        auto& comp = manager.addComponent<TestComponent>(entityId);
        comp.value = 42;

        manager.removeComponent<TestComponent>(entityId);
        REQUIRE_FALSE(manager.hasComponent<TestComponent>(entityId));
        REQUIRE(manager.getComponent<TestComponent>(entityId) == nullptr);
    }

    SECTION("Multiple component types") {
        const size_t entityId = 1;
        
        auto& testComp = manager.addComponent<TestComponent>(entityId);
        testComp.value = 42;
        
        auto& posComp = manager.addComponent<PositionComponent>(entityId);
        posComp.x = 1.0f;
        posComp.y = 2.0f;

        REQUIRE(manager.hasComponent<TestComponent>(entityId));
        REQUIRE(manager.hasComponent<PositionComponent>(entityId));
        
        auto* retrievedTest = manager.getComponent<TestComponent>(entityId);
        REQUIRE(retrievedTest != nullptr);
        REQUIRE(retrievedTest->value == 42);
        
        auto* retrievedPos = manager.getComponent<PositionComponent>(entityId);
        REQUIRE(retrievedPos != nullptr);
        REQUIRE(retrievedPos->x == 1.0f);
        REQUIRE(retrievedPos->y == 2.0f);
    }

    SECTION("Remove all components") {
        const size_t entityId = 1;
        
        manager.addComponent<TestComponent>(entityId);
        manager.addComponent<PositionComponent>(entityId);
        
        manager.removeAllComponents(entityId);
        
        REQUIRE_FALSE(manager.hasComponent<TestComponent>(entityId));
        REQUIRE_FALSE(manager.hasComponent<PositionComponent>(entityId));
    }

    SECTION("Component lifecycle callbacks") {
        const size_t entityId = 1;
        bool addedCalled = false;
        bool removedCalled = false;
        
        manager.setOnComponentAdded([&](size_t id, std::type_index type) {
            addedCalled = true;
            REQUIRE(id == entityId);
            REQUIRE(type == std::type_index(typeid(TestComponent)));
        });
        
        manager.setOnComponentRemoved([&](size_t id, std::type_index type) {
            removedCalled = true;
            REQUIRE(id == entityId);
            REQUIRE(type == std::type_index(typeid(TestComponent)));
        });
        
        manager.addComponent<TestComponent>(entityId);
        REQUIRE(addedCalled);
        
        manager.removeComponent<TestComponent>(entityId);
        REQUIRE(removedCalled);
    }
} 