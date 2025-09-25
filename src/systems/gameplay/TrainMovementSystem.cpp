#include "TrainMovementSystem.h"
#include "components/GameLogicComponents.h"
#include "Logger.h"
#include "Constants.h"
#include <cmath>

TrainMovementSystem::TrainMovementSystem(entt::registry& registry)
    : _registry(registry) {
    LOG_DEBUG("TrainMovementSystem", "TrainMovementSystem created.");
}

void TrainMovementSystem::update(sf::Time dt) {
    const float timeStep = dt.asSeconds();

    auto approachView = _registry.view<TrainMovementComponent, TrainPhysicsComponent, PositionComponent, StationApproachComponent>();
    for (auto entity : approachView) {
        auto &movement = approachView.get<TrainMovementComponent>(entity);
        auto &physics = approachView.get<TrainPhysicsComponent>(entity);
        auto &position = approachView.get<PositionComponent>(entity);
        auto &approach = approachView.get<StationApproachComponent>(entity);
        handleStationApproach(entity, movement, physics, position, approach, timeStep);
    }

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
        const auto &line = _registry.get<LineComponent>(movement.assignedLine);
        
        if (movement.direction == TrainDirection::FORWARD) {
            bool foundNextStop = false;
            for (size_t i = movement.currentSegmentIndex + 1; i < line.points.size(); ++i) {
                if (line.points[i].type == LinePointType::STOP) {
                    foundNextStop = true;
                    break;
                }
            }
            if (!foundNextStop) {
                movement.direction = TrainDirection::BACKWARD;
                LOG_TRACE("TrainMovementSystem", "Train at end of line, reversing to BACKWARD.");
            }
        } else { // BACKWARD
            bool foundNextStop = false;
            for (int i = movement.currentSegmentIndex - 1; i >= 0; --i) {
                if (line.points[i].type == LinePointType::STOP) {
                    foundNextStop = true;
                    break;
                }
            }
            if (!foundNextStop) {
                movement.direction = TrainDirection::FORWARD;
                LOG_TRACE("TrainMovementSystem", "Train at start of line, reversing to FORWARD.");
            }
        }

        movement.state = TrainState::ACCELERATING;
        LOG_TRACE("TrainMovementSystem", "Train state changed to ACCELERATING.");
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
    sf::Vector2f p2 = line.points[movement.currentSegmentIndex].position;

    float one_minus_t = 1.0f - t;
    position.coordinates = one_minus_t * one_minus_t * p0 + 2.0f * one_minus_t * t * p1 + t * t * p2;

    if (physics.currentSpeed <= 0.0f || t >= 1.0f) {
        physics.currentSpeed = 0.0f;
        movement.state = TrainState::STOPPED;
        LOG_TRACE("TrainMovementSystem", "Train state changed to STOPPED.");
        movement.stopTimer = Constants::TRAIN_STOP_DURATION;
        position.coordinates = p2;
        movement.progressOnSegment = 0.0f;
        
        _registry.remove<StationApproachComponent>(entity);
    }
}

void TrainMovementSystem::handleMovement(entt::entity entity, TrainMovementComponent &movement, TrainPhysicsComponent &physics, PositionComponent &position, float timeStep) {
    if (!_registry.valid(movement.assignedLine)) return;
    const auto &line = _registry.get<LineComponent>(movement.assignedLine);
    if (line.points.size() < 2) return;

    int currentPointIndex = movement.currentSegmentIndex;
    int nextPointIndex = (movement.direction == TrainDirection::FORWARD) ? currentPointIndex + 1 : currentPointIndex - 1;

    if (nextPointIndex < 0 || nextPointIndex >= line.points.size()) {
        movement.state = TrainState::STOPPED;
        movement.stopTimer = Constants::TRAIN_STOP_DURATION;
        LOG_WARN("TrainMovementSystem", "Train moved past end of line unexpectedly. Stopping.");
        return;
    }

    sf::Vector2f startPos = line.points[currentPointIndex].position;
    sf::Vector2f endPos = line.points[nextPointIndex].position;

    int segmentIndexForOffset = (movement.direction == TrainDirection::FORWARD) ? currentPointIndex : nextPointIndex;
    if (segmentIndexForOffset >= 0 && segmentIndexForOffset < line.pathOffsets.size()) {
        sf::Vector2f segmentOffset = line.pathOffsets[segmentIndexForOffset];
        startPos += segmentOffset;
        endPos += segmentOffset;
    }

    sf::Vector2f segmentVector = endPos - startPos;
    float segmentLength = std::sqrt(segmentVector.x * segmentVector.x + segmentVector.y * segmentVector.y);
    
    float distanceToNextStop = 0.0f;
    int nextStopIndex = -1;
    if (movement.direction == TrainDirection::FORWARD) {
        distanceToNextStop = (1.0f - movement.progressOnSegment) * segmentLength;
        for (size_t i = nextPointIndex; i < line.points.size(); ++i) {
            if (line.points[i].type == LinePointType::STOP) {
                nextStopIndex = i;
                break;
            }
            if (i + 1 < line.points.size()) {
                sf::Vector2f p1 = line.points[i].position;
                sf::Vector2f p2 = line.points[i+1].position;
                distanceToNextStop += std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
            }
        }
    } else { // BACKWARD
        distanceToNextStop = (1.0f - movement.progressOnSegment) * segmentLength;
        for (int i = nextPointIndex; i >= 0; --i) {
            if (line.points[i].type == LinePointType::STOP) {
                nextStopIndex = i;
                break;
            }
            if (i - 1 >= 0) {
                 sf::Vector2f p1 = line.points[i].position;
                 sf::Vector2f p2 = line.points[i-1].position;
                 distanceToNextStop += std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
            }
        }
    }

    if (movement.state == TrainState::ACCELERATING) {
        physics.currentSpeed += physics.acceleration * timeStep;
        if (physics.currentSpeed >= physics.maxSpeed) {
            physics.currentSpeed = physics.maxSpeed;
            movement.state = TrainState::MOVING;
            LOG_TRACE("TrainMovementSystem", "Train state changed to MOVING.");
        }
    }

    float decelerationDistance = (physics.currentSpeed * physics.currentSpeed) / (2.0f * physics.acceleration);
    if (nextStopIndex != -1 && distanceToNextStop <= decelerationDistance && segmentLength > 0) {
        movement.state = TrainState::DECELERATING;
        LOG_TRACE("TrainMovementSystem", "Train state changed to DECELERATING, approaching stop %d.", nextStopIndex);
        
        auto& approach = _registry.emplace<StationApproachComponent>(entity);
        approach.decelerationDistance = distanceToNextStop;
        approach.decelerationProgress = 0.0f;
        approach.approachCurveStart = position.coordinates;
        approach.approachCurveControl = line.points[nextStopIndex].position;
        
        movement.currentSegmentIndex = nextStopIndex;
        return;
    }

    if (segmentLength > 0.0f) {
        float distanceToTravel = physics.currentSpeed * timeStep;
        movement.progressOnSegment += distanceToTravel / segmentLength;

        if (movement.progressOnSegment >= 1.0f) {
            movement.progressOnSegment = 0.0f;
            movement.currentSegmentIndex = nextPointIndex;
            position.coordinates = endPos;
        } else {
            position.coordinates = startPos + segmentVector * movement.progressOnSegment;
        }
    } else {
        movement.progressOnSegment = 0.0f;
        movement.currentSegmentIndex = nextPointIndex;
    }
}