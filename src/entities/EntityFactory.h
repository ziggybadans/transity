// src/entities/EntityFactory.h
#pragma once
#include "EntityManager.h"
#include "../components/Component.h"

class EntityFactory {
public:
    Entity createPlayer(EntityManager& entityManager);
    // Add methods for other entity types
};
