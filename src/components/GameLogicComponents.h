#pragma once

#include "Constants.h"
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
enum class LinePointType { STOP, CONTROL_POINT };

struct LinePoint {
    LinePointType type;
    sf::Vector2f position;
    entt::entity stationEntity = entt::null;
};

struct LineComponent {
    sf::Color color;
    std::vector<LinePoint> points;
    std::vector<sf::Vector2f> pathOffsets;
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
enum class TrainState { STOPPED, ACCELERATING, MOVING, DECELERATING };

enum class TrainDirection { FORWARD, BACKWARD };

// A tag to identify train entities
struct TrainTag {};

// Manages the train's state machine and progress on a line
struct TrainMovementComponent {
    TrainState state = TrainState::STOPPED;
    TrainDirection direction = TrainDirection::FORWARD;
    entt::entity assignedLine;
    int currentSegmentIndex = 0;
    float progressOnSegment = 0.0f;
    float stopTimer = 2.0f;
};

// Manages the train's physics properties
struct TrainPhysicsComponent {
    float maxSpeed = Constants::TRAIN_MAX_SPEED;
    float currentSpeed = 50.0f;
    float acceleration = Constants::TRAIN_ACCELERATION;
};

// Manages passenger capacity and load
struct TrainCapacityComponent {
    int capacity = Constants::TRAIN_CAPACITY;
    int currentLoad = 0;  // Add this line
};

// A temporary component added when a train begins its station approach
struct StationApproachComponent {
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

// A component to manage the passenger spawn animation.
struct PassengerSpawnAnimationComponent {
    float progress = 0.0f;
    float duration = 1.0f;
    entt::entity originCity;
    entt::entity destinationCity;
};