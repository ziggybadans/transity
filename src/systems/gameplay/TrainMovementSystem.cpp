// src/systems/gameplay/TrainMovementSystem.cpp

#include "TrainMovementSystem.h"
#include "components/GameLogicComponents.h"
#include "core/ServiceLocator.h"
#include "Logger.h"
#include <cmath>

const float TrainComponent::STOP_DURATION = 2.0f;

TrainMovementSystem::TrainMovementSystem(ServiceLocator &serviceLocator)
    : _registry(serviceLocator.registry) {
    LOG_DEBUG("TrainMovementSystem", "TrainMovementSystem created.");
}

void TrainMovementSystem::update(sf::Time dt) {
    auto view = _registry.view<TrainComponent, PositionComponent>();
    const float timeStep = dt.asSeconds();

    for (auto entity : view) {
        auto &train = view.get<TrainComponent>(entity);
        auto &position = view.get<PositionComponent>(entity);

        if (!_registry.valid(train.assignedLine)) {
            continue;
        }
        const auto &line = _registry.get<LineComponent>(train.assignedLine);
        if (line.stops.size() < 2) {
            continue;
        }

        int nextStopIndex;
        if (train.direction == TrainDirection::FORWARD) {
            nextStopIndex = train.currentSegmentIndex + 1;
        } else { // BACKWARD
            nextStopIndex = train.currentSegmentIndex - 1;
        }

        if (train.state != TrainState::STOPPED && (nextStopIndex < 0 || nextStopIndex >= line.stops.size())) {
            // A moving train has reached a terminal. Force it to decelerate to a stop.
            // The arrival logic will then handle turning around.
            train.state = TrainState::DECELERATING;
            train.currentSpeed = 0.0f;
        }

        // If the train is stopped, we don't need to calculate its next move yet.
        if (train.state == TrainState::STOPPED) {
            train.stopTimer -= timeStep;
            if (train.stopTimer <= 0.0f) {
                train.state = TrainState::ACCELERATING;
            }
            continue; // Skip movement calculations for this frame
        }

        entt::entity currentStopEntity = line.stops[train.currentSegmentIndex];
        entt::entity nextStopEntity = line.stops[nextStopIndex];

        if (!_registry.valid(currentStopEntity) || !_registry.valid(nextStopEntity)) {
            break;
        }

        const auto &currentStopPos = _registry.get<PositionComponent>(currentStopEntity).coordinates;
        const auto &nextStopPos = _registry.get<PositionComponent>(nextStopEntity).coordinates;
        sf::Vector2f segmentVector = nextStopPos - currentStopPos;
        float segmentLength = std::sqrt(segmentVector.x * segmentVector.x + segmentVector.y * segmentVector.y);
        float distanceToNextStop = (1.0f - train.progressOnSegment) * segmentLength;

        // --- State Machine ---
        switch (train.state) {
        case TrainState::ACCELERATING: {
            train.currentSpeed += train.acceleration * timeStep;
            if (train.currentSpeed >= train.maxSpeed) {
                train.currentSpeed = train.maxSpeed;
                train.state = TrainState::MOVING;
            }
            float decelerationDistance = (train.currentSpeed * train.currentSpeed) / (2.0f * train.acceleration);
            if (distanceToNextStop <= decelerationDistance) {
                train.state = TrainState::DECELERATING;
            }
            break;
        }

        case TrainState::MOVING: {
            float decelerationDistance = (train.currentSpeed * train.currentSpeed) / (2.0f * train.acceleration);
            if (distanceToNextStop <= decelerationDistance) {
                train.state = TrainState::DECELERATING;
            }
            break;
        }

        case TrainState::DECELERATING:
            train.currentSpeed -= train.acceleration * timeStep;
            if (train.currentSpeed <= 0.0f) {
                train.currentSpeed = 0.0f;
                train.state = TrainState::STOPPED;
                train.stopTimer = TrainComponent::STOP_DURATION;

                position.coordinates = nextStopPos;
                train.progressOnSegment = 0.0f;
                train.currentSegmentIndex = nextStopIndex;

                // Reverse direction at terminal stations
                if (train.direction == TrainDirection::FORWARD && train.currentSegmentIndex >= line.stops.size() - 1) {
                    train.direction = TrainDirection::BACKWARD;
                } else if (train.direction == TrainDirection::BACKWARD && train.currentSegmentIndex <= 0) {
                    train.direction = TrainDirection::FORWARD;
                }
            }
            break;
        }

        // --- Position Update ---
        if (train.state != TrainState::STOPPED) {
            if (segmentLength > 0.0f) {
                float distanceToTravel = train.currentSpeed * timeStep;
                train.progressOnSegment += distanceToTravel / segmentLength;

                if (train.progressOnSegment >= 1.0f) {
                    // Reached the destination station. Clamp position and progress.
                    // The DECELERATING state handler is now responsible for bringing the train to a full stop.
                    train.progressOnSegment = 1.0f;
                    position.coordinates = nextStopPos;

                    // If we arrived but weren't slowing down, force it.
                    // This handles short segments where deceleration might not have triggered.
                    if (train.state != TrainState::DECELERATING) {
                        train.state = TrainState::DECELERATING;
                    }
                } else {
                    // In transit, update position via interpolation.
                    position.coordinates = currentStopPos + segmentVector * train.progressOnSegment;
                }
            }
        }
    }
}