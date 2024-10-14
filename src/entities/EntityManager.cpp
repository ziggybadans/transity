// src/entities/EntityManager.cpp
#include "EntityManager.h"

Entity EntityManager::createEntity() {
    EntityID id;
    if (!availableIDs.empty()) {
        id = availableIDs.front();
        availableIDs.pop();
    }
    else {
        id = nextID++;
    }
    return Entity(id);
}

void EntityManager::destroyEntity(EntityID id) {
    availableIDs.push(id);
    // Remove components associated with the entity
}
