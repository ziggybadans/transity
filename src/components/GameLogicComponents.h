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
    std::vector<entt::entity> waitingPassengers;
};

// A component for line entities.
enum class LinePointType { STOP, CONTROL_POINT };

struct StopInfo {
    entt::entity stationEntity;
    float distanceAlongCurve;
};

struct SnapInfo {
    entt::entity snappedToEntity;
    size_t snappedToPointIndex;
};

struct LinePoint {
    LinePointType type;
    sf::Vector2f position;
    entt::entity stationEntity = entt::null;
    std::optional<SnapInfo> snapInfo;
    float snapSide = 0.f; // Add this. 0 for center, -1 for left, 1 for right.
};

// Represents a segment between two control points that is shared by multiple lines.
struct SharedSegment {
    // The entities of the lines that share this segment.
    std::vector<entt::entity> lines;
};

// A global context structure to hold all shared segments in the game world.
struct SharedSegmentsContext {
    // A map where the key is a pair of point indices (from a canonical line)
    // and the value is the shared segment information.
    std::map<std::pair<size_t, size_t>, SharedSegment> segments;
};

struct LineComponent {
    sf::Color color;
    std::vector<LinePoint> points;
    std::vector<sf::Vector2f> pathOffsets;
    std::vector<sf::Vector2f> curvePoints;
    std::vector<size_t> curveSegmentIndices;
    std::vector<StopInfo> stops;  // Add this line
    float totalDistance = 0.0f;
    Thickness thickness = {Constants::DEFAULT_LINE_THICKNESS};

    // A map where the key is the segment index and the value is a pointer
    // to the corresponding SharedSegment in the global context.
    std::map<size_t, SharedSegment*> sharedSegments;
};

// A component to manage the state of line editing.
struct LineEditingComponent {
    std::optional<size_t> selectedPointIndex;
    std::optional<size_t> draggedPointIndex;
    std::optional<sf::Vector2f> originalPointPosition;
    std::optional<sf::Vector2f> snapPosition;
    std::optional<SnapInfo> snapInfo;
    float snapSide = 0.f;
    std::optional<sf::Vector2f> snapTangent;
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
    float distanceAlongCurve = 0.0f;  // Change this
    float stopTimer = Constants::TRAIN_STOP_DURATION;
};

// Manages the train's physics properties
struct TrainPhysicsComponent {
    float maxSpeed = Constants::TRAIN_MAX_SPEED;
    float currentSpeed = Constants::TRAIN_DEFAULT_INITIAL_SPEED;
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
    float duration = Constants::PASSENGER_SPAWN_ANIMATION_DURATION;
    entt::entity originCity;
    entt::entity destinationCity;
};