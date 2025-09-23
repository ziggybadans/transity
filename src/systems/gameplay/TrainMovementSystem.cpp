// src/systems/gameplay/TrainMovementSystem.cpp

#include "TrainMovementSystem.h"
#include "components/GameLogicComponents.h"
#include "core/ServiceLocator.h"
#include "Logger.h"
#include <cmath>

TrainMovementSystem::TrainMovementSystem(ServiceLocator &serviceLocator)
    : _registry(serviceLocator.registry) {
    LOG_DEBUG("TrainMovementSystem", "TrainMovementSystem created.");
}

void TrainMovementSystem::update(sf::Time dt) {
    const float timeStep = dt.asSeconds();

    // Use a view to find trains that are approaching a station and handle them first.
    auto approachView = _registry.view<TrainMovementComponent, TrainPhysicsComponent, PositionComponent, StationApproachComponent>();
    for (auto entity : approachView) {
        auto &movement = approachView.get<TrainMovementComponent>(entity);
        auto &physics = approachView.get<TrainPhysicsComponent>(entity);
        auto &position = approachView.get<PositionComponent>(entity);
        auto &approach = approachView.get<StationApproachComponent>(entity);
        handleStationApproach(entity, movement, physics, position, approach, timeStep);
    }

    // Use a second view to handle all other trains (stopped, accelerating, moving).
    // Exclude trains that are already being handled by the approach logic.
    auto mainView = _registry.view<TrainTag, TrainMovementComponent, TrainPhysicsComponent, PositionComponent>(entt::exclude<StationApproachComponent>);
    for (auto entity : mainView) {
        auto &movement = mainView.get<TrainMovementComponent>(entity);
        if (movement.state == TrainState::STOPPED) {
            handleStoppedState(movement, timeStep);
        } else {
            auto &physics = mainView.get<TrainPhysicsComponent>(entity);
            auto &position = mainView.get<PositionComponent>(entity);
            handleMovement(entity, movement, physics, position, timeStep);
        }
    }
}

void TrainMovementSystem::handleStoppedState(TrainMovementComponent &movement, float timeStep) {
    movement.stopTimer -= timeStep;
    if (movement.stopTimer <= 0.0f) {
        movement.state = TrainState::ACCELERATING;
    }
}

void TrainMovementSystem::handleStationApproach(entt::entity entity, TrainMovementComponent &movement, TrainPhysicsComponent &physics, PositionComponent &position, StationApproachComponent &approach, float timeStep) {
    const auto &line = _registry.get<LineComponent>(movement.assignedLine);
    
    physics.currentSpeed -= physics.acceleration * timeStep;
    approach.decelerationProgress += physics.currentSpeed * timeStep;
    
    float t = (approach.decelerationDistance > 0) ? (approach.decelerationProgress / approach.decelerationDistance) : 1.0f;
    if (t > 1.0f) t = 1.0f;

    sf::Vector2f p0 = approach.approachCurveStart;
    sf::Vector2f p1 = approach.approachCurveControl;
    sf::Vector2f p2 = _registry.get<PositionComponent>(line.stops[movement.currentSegmentIndex]).coordinates;

    float one_minus_t = 1.0f - t;
    position.coordinates = one_minus_t * one_minus_t * p0 + 2.0f * one_minus_t * t * p1 + t * t * p2;

    if (physics.currentSpeed <= 0.0f || t >= 1.0f) {
        physics.currentSpeed = 0.0f;
        movement.state = TrainState::STOPPED;
        movement.stopTimer = TrainMovementComponent::STOP_DURATION;
        position.coordinates = p2;
        movement.progressOnSegment = 0.0f;
        
        // The approach is complete, so remove the component.
        _registry.remove<StationApproachComponent>(entity);

        if (movement.direction == TrainDirection::FORWARD && movement.currentSegmentIndex >= line.stops.size() - 1) {
            movement.direction = TrainDirection::BACKWARD;
        } else if (movement.direction == TrainDirection::BACKWARD && movement.currentSegmentIndex <= 0) {
            movement.direction = TrainDirection::FORWARD;
        }
    }
}

void TrainMovementSystem::handleMovement(entt::entity entity, TrainMovementComponent &movement, TrainPhysicsComponent &physics, PositionComponent &position, float timeStep) {
    if (!_registry.valid(movement.assignedLine)) return;
    const auto &line = _registry.get<LineComponent>(movement.assignedLine);
    if (line.stops.size() < 2) return;

    int nextStopIndex = (movement.direction == TrainDirection::FORWARD) ? movement.currentSegmentIndex + 1 : movement.currentSegmentIndex - 1;
    if (nextStopIndex < 0 || nextStopIndex >= line.stops.size()) return;

    int segmentIndexForOffset = (movement.direction == TrainDirection::FORWARD) ? movement.currentSegmentIndex : movement.currentSegmentIndex - 1;

    sf::Vector2f startPos = _registry.get<PositionComponent>(line.stops[movement.currentSegmentIndex]).coordinates;
    sf::Vector2f endPos;
    sf::Vector2f segmentOffset(0.f, 0.f);

    if (segmentIndexForOffset >= 0 && segmentIndexForOffset < line.pathOffsets.size()) {
        segmentOffset = line.pathOffsets[segmentIndexForOffset];
    }
    endPos = _registry.get<PositionComponent>(line.stops[nextStopIndex]).coordinates + segmentOffset;

    sf::Vector2f segmentVector = endPos - startPos;
    float segmentLength = std::sqrt(segmentVector.x * segmentVector.x + segmentVector.y * segmentVector.y);
    float distanceToNextStop = (1.0f - movement.progressOnSegment) * segmentLength;

    if (movement.state == TrainState::ACCELERATING) {
        physics.currentSpeed += physics.acceleration * timeStep;
        if (physics.currentSpeed >= physics.maxSpeed) {
            physics.currentSpeed = physics.maxSpeed;
            movement.state = TrainState::MOVING;
        }
    }

    float decelerationDistance = (physics.currentSpeed * physics.currentSpeed) / (2.0f * physics.acceleration);
    if (distanceToNextStop <= decelerationDistance && segmentLength > 0) {
        movement.state = TrainState::DECELERATING;
        
        // Add the StationApproachComponent to begin the curved approach.
        auto& approach = _registry.emplace<StationApproachComponent>(entity);
        approach.decelerationDistance = distanceToNextStop;
        approach.decelerationProgress = 0.0f;
        approach.approachCurveStart = position.coordinates;
        approach.approachCurveControl = endPos;
        
        // Update the target segment index now, so the approach system has the right target.
        movement.currentSegmentIndex = nextStopIndex;
        return;
    }

    // Standard linear movement
    if (segmentLength > 0.0f) {
        float distanceToTravel = physics.currentSpeed * timeStep;
        movement.progressOnSegment += distanceToTravel / segmentLength;

        if (movement.progressOnSegment >= 1.0f) {
            movement.progressOnSegment = 0.0f;
            movement.currentSegmentIndex = nextStopIndex;
            position.coordinates = endPos;
        } else {
            position.coordinates = startPos + segmentVector * movement.progressOnSegment;
        }
    } else {
        movement.progressOnSegment = 0.0f;
        movement.currentSegmentIndex = nextStopIndex;
    }
}