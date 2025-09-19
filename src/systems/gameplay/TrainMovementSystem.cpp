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

        if (train.currentSegmentIndex >= line.stops.size()) {
            train.currentSegmentIndex = 0;
        }
        int nextStopIndex = (train.currentSegmentIndex + 1) % line.stops.size();
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
        case TrainState::STOPPED:
            train.stopTimer -= timeStep;
            if (train.stopTimer <= 0.0f) {
                train.state = TrainState::ACCELERATING;
            }
            break;

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