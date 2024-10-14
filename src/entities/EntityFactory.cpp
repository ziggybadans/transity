// src/entities/EntityFactory.cpp
#include "EntityFactory.h"

Entity EntityFactory::createPlayer(EntityManager& entityManager) {
    Entity player = entityManager.createEntity();
    // Attach Position and Velocity components
    // e.g., componentManager.addComponent<Position>(player.getID(), Position{0.f, 0.f});
    return player;
}
