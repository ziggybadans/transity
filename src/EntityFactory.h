#pragma once

#include <entt/entt.hpp>
#include <SFML/System.hpp> // Changed from Vector2f.hpp for broader include
#include <string>

// Assuming Components.h defines PositionComponent, NameComponent, 
// StationTag, RenderableComponent and includes necessary SFML headers
// for sf::Color and shape properties if RenderableComponent uses them directly.
#include "Components.h" 

class EntityFactory {
public:
    // Constructor takes a reference to the game's entity registry
    EntityFactory(entt::registry& registry);

    // Creates a station entity with the given position and name
    // Returns the ID of the created entity
    entt::entity createStation(const sf::Vector2f& position, const std::string& name);

    // Future entity creation methods can be added here:
    // entt::entity createTrain(...);
    // entt::entity createTrackSegment(...);

private:
    entt::registry& m_registry; // Reference to the external registry
};