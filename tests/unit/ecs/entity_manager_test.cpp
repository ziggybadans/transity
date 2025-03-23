#include <catch2/catch_test_macros.hpp>
#include "transity/ecs/entity_manager.hpp"
#include <limits>

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

    SECTION("Entity equality operator") {
        auto entity1 = manager.createEntity();
        auto entity2 = manager.createEntity();
        auto entity1Copy = entity1;

        REQUIRE(entity1 == entity1Copy);
        REQUIRE_FALSE(entity1 == entity2);

        // Same ID but different version should not be equal
        Entity differentVersion{entity1.id, entity1.version + 1};
        REQUIRE_FALSE(entity1 == differentVersion);
    }

    SECTION("Rapid creation and destruction") {
        std::vector<Entity> entities;
        const size_t numEntities = 1000;

        // Create many entities
        for (size_t i = 0; i < numEntities; ++i) {
            entities.push_back(manager.createEntity());
        }
        REQUIRE(manager.getEntityCount() == numEntities);

        // Destroy all entities
        for (const auto& entity : entities) {
            REQUIRE(manager.destroyEntity(entity));
        }
        REQUIRE(manager.getEntityCount() == 0);

        // Create new entities, should reuse IDs
        for (size_t i = 0; i < numEntities; ++i) {
            auto newEntity = manager.createEntity();
            REQUIRE(newEntity.id < numEntities);
            REQUIRE(newEntity.version > 1);
        }
    }

    SECTION("Entity version sequence") {
        // Create initial entity
        auto entity1 = manager.createEntity();
        REQUIRE(entity1.version == 1);  // Initial version is 1

        // Destroy it, which increments version
        REQUIRE(manager.destroyEntity(entity1));

        // Create new entity with same ID, should have incremented version
        auto entity2 = manager.createEntity();
        REQUIRE(entity2.id == entity1.id);
        REQUIRE(entity2.version == 3);  // Version incremented on destroy and create

        // Destroy and recreate again
        REQUIRE(manager.destroyEntity(entity2));
        auto entity3 = manager.createEntity();
        REQUIRE(entity3.id == entity1.id);
        REQUIRE(entity3.version == 5);  // Version incremented twice more
    }
} 