// src/entities/Entity.h
#pragma once
#include <cstdint>  // Include for fixed-width integer types

using EntityID = std::uint32_t;  // Define EntityID as a 32-bit unsigned integer for unique entity identification

class Entity {
public:
    // Constructor: Initializes the entity with a unique ID
    Entity(EntityID id) : id(id) {}

    // Getter function to retrieve the entity's unique ID
    EntityID getID() const { return id; }

private:
    EntityID id;  // Unique identifier for the entity
};

// Summary:
// The Entity class represents a basic game entity with a unique identifier. The EntityID is used to differentiate
// each entity within the game. This class serves as a base that can be extended by other entities with more
// complex behaviors or attributes.