// src/entities/EntityFactory.h
#pragma once

#include "EntityManager.h"
#include "../components/Component.h"  // Include to attach components to entities
#include <memory>

class EntityFactory {
public:
    // Method to create a player entity and add it to the EntityManager
    Entity createPlayer(EntityManager& entityManager);

    // Create terrain entity (Land or Water)
    Entity createTerrain(EntityManager& entityManager, float x, float y, Terrain::Type type, float height = 0.0f);
};
