// src/entities/EntityManager.cpp
#include "EntityManager.h"

// Creates a new entity, reusing an available ID if possible, or generating a new one if none are available
Entity EntityManager::createEntity() {
    EntityID id;
    if (!availableIDs.empty()) { // If there are reusable IDs, use one of them
        id = availableIDs.front();
        availableIDs.pop();
    }
    else { // Otherwise, generate a new ID
        id = nextID++;
    }
    return Entity(id); // Return the new entity with the assigned ID
}

// Destroys an entity by pushing its ID back to the available queue for reuse
void EntityManager::destroyEntity(EntityID id) {
    availableIDs.push(id); // Mark the entity ID as available for reuse
    // Remove components associated with the entity (not implemented here)
    // This would typically involve using a ComponentManager or similar system to remove all components tied to this entity ID
}

// Summary:
// The EntityManager implementation provides the logic for creating and destroying entities, reusing IDs to optimize resource management.
// When an entity is destroyed, its ID is pushed to a queue for future reuse, ensuring efficient use of IDs without fragmentation.