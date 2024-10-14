// src/systems/MovementSystem.h
#pragma once
#include "../entities/EntityManager.h"
#include "../components/Component.h"

class MovementSystem {
public:
    void update(EntityManager& entityManager, float deltaTime);
};
