// src/entities/Entity.h
#pragma once
#include <cstdint>

using EntityID = std::uint32_t;

class Entity {
public:
    Entity(EntityID id) : id(id) {}
    EntityID getID() const { return id; }

private:
    EntityID id;
};
