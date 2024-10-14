// src/entities/EntityManager.h
#pragma once
#include "Entity.h"
#include <queue>

class EntityManager {
public:
    Entity createEntity();
    void destroyEntity(EntityID id);

private:
    std::queue<EntityID> availableIDs;
    EntityID nextID = 0;
};
