#include <catch2/catch_test_macros.hpp>
#include "transity/ecs/entity_manager.hpp"

using namespace transity::ecs;

TEST_CASE("EntityManager basic functionality", "[ecs]") {
    EntityManager manager;

    SECTION("Creating entities") {
        auto entity1 = manager.createEntity();
        auto entity2 = manager.createEntity();

        REQUIRE(entity1.id == 0);
        REQUIRE(entity1.version == 1);
        REQUIRE(entity2.id == 1);
        REQUIRE(entity2.version == 1);
        REQUIRE(manager.getEntityCount() == 2);
    }

    SECTION("Entity validation") {
        auto entity = manager.createEntity();
        REQUIRE(manager.isValid(entity));

        // Invalid entity should not be valid
        Entity invalidEntity{999, 1};
        REQUIRE_FALSE(manager.isValid(invalidEntity));
    }

    SECTION("Destroying entities") {
        auto entity = manager.createEntity();
        REQUIRE(manager.isValid(entity));
        REQUIRE(manager.getEntityCount() == 1);

        REQUIRE(manager.destroyEntity(entity));
        REQUIRE_FALSE(manager.isValid(entity));
        REQUIRE(manager.getEntityCount() == 0);

        // Destroying invalid entity should fail
        REQUIRE_FALSE(manager.destroyEntity(entity));
    }

    SECTION("Entity recycling") {
        auto entity1 = manager.createEntity();
        REQUIRE(manager.destroyEntity(entity1));

        auto entity2 = manager.createEntity();
        REQUIRE(entity2.id == entity1.id);
        REQUIRE(entity2.version > entity1.version);
        REQUIRE(manager.isValid(entity2));
        REQUIRE_FALSE(manager.isValid(entity1));
    }

    SECTION("Active entities list") {
        auto entity1 = manager.createEntity();
        auto entity2 = manager.createEntity();
        auto entity3 = manager.createEntity();

        const auto& activeEntities = manager.getActiveEntities();
        REQUIRE(activeEntities.size() == 3);
        REQUIRE(std::find_if(activeEntities.begin(), activeEntities.end(),
            [entity1](const Entity& e) { return e == entity1; }) != activeEntities.end());
        REQUIRE(std::find_if(activeEntities.begin(), activeEntities.end(),
            [entity2](const Entity& e) { return e == entity2; }) != activeEntities.end());
        REQUIRE(std::find_if(activeEntities.begin(), activeEntities.end(),
            [entity3](const Entity& e) { return e == entity3; }) != activeEntities.end());

        manager.destroyEntity(entity2);
        REQUIRE(activeEntities.size() == 2);
        REQUIRE(std::find_if(activeEntities.begin(), activeEntities.end(),
            [entity2](const Entity& e) { return e == entity2; }) == activeEntities.end());
    }
} 