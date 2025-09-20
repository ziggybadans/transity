#pragma once

#include "StrongTypes.h"
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <vector>

// The position of an entity in the world.
struct PositionComponent {
    sf::Vector2f coordinates;
};

// A component for city entities.
struct CityComponent {
    std::vector<entt::entity> connectedLines;
    std::vector<entt::entity> waitingPassengers;
};

// A component for line entities.
struct LineComponent {
    sf::Color color;
    std::vector<entt::entity> stops;
    std::vector<sf::Vector2f> pathPoints;
    std::vector<sf::Vector2f> pathOffsets; // Add this line
    Thickness thickness = {5.0f};
};

// A component that makes an entity clickable.
struct ClickableComponent {
    Radius boundingRadius;
};

// A tag for stations that are part of a line currently being created.
struct ActiveLineStationTag {
    StationOrder order = {0};
};

// Enum for the state of a train
enum class TrainState {
    STOPPED,
    ACCELERATING,
    MOVING,
    DECELERATING
};

enum class TrainDirection {
    FORWARD,
    BACKWARD
};

// A component for train entities.
struct TrainComponent {
    entt::entity assignedLine;
    int currentSegmentIndex = 0;
    float progressOnSegment = 0.0f;
    TrainDirection direction = TrainDirection::FORWARD;
    float maxSpeed = 100.0f;
    float currentSpeed = 50.0f; // World units per second
    float acceleration = 25.0f;
    TrainState state = TrainState::STOPPED;
    float stopTimer = 2.0f;
    static const float STOP_DURATION; // Changed from static constexpr
    int capacity = 20;
    int currentLoad = 0;
    std::vector<entt::entity> passengers;

    // Fields for curved station approach
    bool isApproachingStation = false;
    sf::Vector2f approachCurveStart;
    sf::Vector2f approachCurveControl;
    float decelerationProgress = 0.0f;
    float decelerationDistance = 0.0f;
};

// A tag to mark an entity as selected.
struct SelectedComponent {};

// A component for storing a displayable name for an entity.
struct NameComponent {
    std::string name;
};