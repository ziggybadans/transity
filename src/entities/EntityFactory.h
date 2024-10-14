// src/entities/EntityFactory.h
#pragma once
#include "EntityManager.h"  // Include for managing entities within the game
#include "../components/Component.h"  // Include to potentially attach components to entities during creation

class EntityFactory {
public:
    // Method to create a player entity and add it to the EntityManager
    Entity createPlayer(EntityManager& entityManager);

    // Add methods for other entity types (e.g., enemies, NPCs)
    // Entity createEnemy(EntityManager& entityManager);
    // Entity createNPC(EntityManager& entityManager);
};

// Summary:
// The EntityFactory class provides methods for creating various types of game entities, such as players, enemies, or NPCs.
// These methods leverage the EntityManager to manage and store the created entities, and allow for easy entity instantiation
// and configuration, including the addition of components for specific behaviors or attributes.