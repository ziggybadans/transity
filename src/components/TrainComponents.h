#pragma once

#include "Constants.h"
#include "StrongTypes.h"
#include <SFML/Graphics.hpp>
#include <entt/entt.hpp>
#include <optional>
#include <vector>

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
    float distanceAlongCurve = 0.0f;  // Change this
    float stopTimer = Constants::TRAIN_STOP_DURATION;
};

// Manages the train's physics properties
struct TrainPhysicsComponent {
    float maxSpeed = Constants::TRAIN_MAX_SPEED;
    float currentSpeed = 0.0f;
    float acceleration = Constants::TRAIN_ACCELERATION;
};

// Manages passenger capacity and load
struct TrainCapacityComponent {
    int capacity = Constants::TRAIN_CAPACITY;
    int currentLoad = 0;  // Add this line
};

// A component added to a train when it is stopped at a station.
struct AtStationComponent {
    entt::entity stationEntity;
};

// A temporary component added when a train begins its station approach
struct StationApproachComponent {
    sf::Vector2f approachCurveStart;
    sf::Vector2f approachCurveControl;
    float decelerationProgress = 0.0f;
    float decelerationDistance = 0.0f;
};