#pragma once

#include "Constants.h"
#include "StrongTypes.h"
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <optional>
#include <vector>

// The position of an entity in the world.
struct PositionComponent {
    sf::Vector2f coordinates;
};

// Enum for the type of city.
enum class CityType { CAPITAL, TOWN, SUBURB };

// A component for city entities.
struct CityComponent {
    CityType type;
    std::vector<entt::entity> connectedLines;
};

// A component that makes an entity clickable.
struct ClickableComponent {
    Radius boundingRadius;
};

// A tag to mark an entity as selected.
struct SelectedComponent {};

// A component for storing a displayable name for an entity.
struct NameComponent {
    std::string name;
};

// A component to manage the passenger spawn animation.
struct PassengerSpawnAnimationComponent {
    float progress = 0.0f;
    float duration = Constants::PASSENGER_SPAWN_ANIMATION_DURATION;
    entt::entity originCity;
    entt::entity destinationCity;
};

// A component for storing the game score.
struct GameScoreComponent {
    int score = 0;
};