// src/systems/MovementSystem.h
#pragma once
#include "../entities/EntityManager.h"  // Include for managing entities within the system
#include "../components/Component.h"    // Include to access components, such as Position or Velocity

class MovementSystem {
public:
    // Updates the positions of entities with movement components based on their velocity.
    void update(EntityManager& entityManager, float deltaTime);
};

// Summary:
// The MovementSystem class is responsible for updating the positions of entities that have movement-related components.
// It processes entities by accessing their Position and Velocity components and adjusts their positions accordingly
// during each frame, based on the elapsed time (deltaTime). This system facilitates smooth entity movement within the game.