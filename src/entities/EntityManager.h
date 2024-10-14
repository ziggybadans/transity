// src/entities/EntityManager.h
#pragma once
#include "Entity.h"  // Include Entity definition to manage entities
#include <queue>      // Include for managing available entity IDs

class EntityManager {
public:
    // Creates a new entity and returns it, either by reusing an available ID or by generating a new one
    Entity createEntity();

    // Destroys an entity by marking its ID as available for reuse
    void destroyEntity(EntityID id);

private:
    std::queue<EntityID> availableIDs;  // Queue to hold reusable entity IDs to avoid ID fragmentation
    EntityID nextID = 0;  // The next ID to be used if no reusable IDs are available
};

// Summary:
// The EntityManager class is responsible for managing the lifecycle of entities, including creating and destroying them.
// It maintains a queue of available IDs for reuse, which helps to avoid gaps in the entity ID sequence and improves efficiency.