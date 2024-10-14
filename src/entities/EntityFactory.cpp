// src/entities/EntityFactory.cpp
#include "EntityFactory.h"

// Method to create a player entity and add it to the EntityManager
Entity EntityFactory::createPlayer(EntityManager& entityManager) {
    Entity player = entityManager.createEntity(); // Create a new entity and assign it as the player

    // Attach Position and Velocity components to the player entity
    // e.g., componentManager.addComponent<Position>(player.getID(), Position{0.f, 0.f});
    // This allows the player to have positional data and potentially movement capabilities

    return player; // Return the created player entity
}

// Summary:
// The EntityFactory::createPlayer method is responsible for creating a player entity using the EntityManager.
// Components such as Position or Velocity can be attached to the player to define its attributes and behaviors.
// This function serves as the basis for instantiating player-specific logic within the game.