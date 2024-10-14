// src/entities/EntityFactory.cpp
#include "EntityFactory.h"

// Example implementation for player creation (assuming player has Position and Velocity)
Entity EntityFactory::createPlayer(EntityManager& entityManager) {
    Entity player = entityManager.createEntity();
    Position pos = { 0.0f, 0.0f };
    Velocity vel = { 0.0f, 0.0f };
    entityManager.addComponent<Position>(player.getID(), pos);
    entityManager.addComponent<Velocity>(player.getID(), vel);
    // Add other player-specific components here
    return player;
}

Entity EntityFactory::createTerrain(EntityManager& entityManager, float x, float y, Terrain::Type type, float height) {
    Entity terrain = entityManager.createEntity();
    Position pos = { x, y };
    Terrain terr = { type };
    Height h = { height };

    entityManager.addComponent<Position>(terrain.getID(), pos);
    entityManager.addComponent<Terrain>(terrain.getID(), terr);
    if (type == Terrain::Type::Land) {
        entityManager.addComponent<Height>(terrain.getID(), h);
    }
    // Water might not need a Height component or could have a default value
    return terrain;
}
