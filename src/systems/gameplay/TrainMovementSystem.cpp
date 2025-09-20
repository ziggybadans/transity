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

        // Handle stopped state
        if (train.state == TrainState::STOPPED) {
            train.stopTimer -= timeStep;
            if (train.stopTimer <= 0.0f) {
                train.state = TrainState::ACCELERATING;
            }
            continue;
        }

        // Determine current segment and next stop
        int nextStopIndex;
        int segmentIndexForOffset = -1;
        if (train.direction == TrainDirection::FORWARD) {
            nextStopIndex = train.currentSegmentIndex + 1;
            segmentIndexForOffset = train.currentSegmentIndex;
        } else { // BACKWARD
            nextStopIndex = train.currentSegmentIndex - 1;
            segmentIndexForOffset = train.currentSegmentIndex - 1;
        }
        
        // Get segment start and end positions with offsets
        const auto &currentStopPos = _registry.get<PositionComponent>(line.stops[train.currentSegmentIndex]).coordinates;
        
        sf::Vector2f segmentOffset(0.f, 0.f);
        if (segmentIndexForOffset >= 0 && segmentIndexForOffset < line.pathOffsets.size()) {
            segmentOffset = line.pathOffsets[segmentIndexForOffset];
        }
        
        sf::Vector2f startPos = currentStopPos + segmentOffset;
        sf::Vector2f endPos;

        if (nextStopIndex >= 0 && nextStopIndex < line.stops.size()) {
            const auto &nextStopPos = _registry.get<PositionComponent>(line.stops[nextStopIndex]).coordinates;
            endPos = nextStopPos + segmentOffset;
        } else {
            // At a terminal, the "end" of the segment is the station itself, but offset.
            int terminalIndex = (train.direction == TrainDirection::FORWARD) ? line.stops.size() - 1 : 0;
            const auto& terminalPos = _registry.get<PositionComponent>(line.stops[terminalIndex]).coordinates;
            endPos = terminalPos + segmentOffset;
        }

        sf::Vector2f segmentVector = endPos - startPos;
        float segmentLength = std::sqrt(segmentVector.x * segmentVector.x + segmentVector.y * segmentVector.y);
        float distanceToNextStop = (1.0f - train.progressOnSegment) * segmentLength;

        // --- State Machine ---
        switch (train.state) {
        case TrainState::ACCELERATING:
            train.currentSpeed += train.acceleration * timeStep;
            if (train.currentSpeed >= train.maxSpeed) {
                train.currentSpeed = train.maxSpeed;
                train.state = TrainState::MOVING;
            }
            // Fall-through to check for deceleration
        case TrainState::MOVING: {
            float decelerationDistance = (train.currentSpeed * train.currentSpeed) / (2.0f * train.acceleration);
            if (distanceToNextStop <= decelerationDistance && segmentLength > 0) {
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

                position.coordinates = endPos;
                train.progressOnSegment = 0.0f;
                
                if (nextStopIndex >= 0 && nextStopIndex < line.stops.size()) {
                    train.currentSegmentIndex = nextStopIndex;
                } else {
                    // Arrived at terminal, reverse direction
                    if (train.direction == TrainDirection::FORWARD) {
                        train.currentSegmentIndex = line.stops.size() - 1;
                        train.direction = TrainDirection::BACKWARD;
                    } else {
                        train.currentSegmentIndex = 0;
                        train.direction = TrainDirection::FORWARD;
                    }
                }
            }
            break;
        case TrainState::STOPPED: break; // Already handled
        }

        // --- Position Update ---
        if (train.state != TrainState::STOPPED) {
            if (segmentLength > 0.0f) {
                float distanceToTravel = train.currentSpeed * timeStep;
                train.progressOnSegment += distanceToTravel / segmentLength;

                if (train.progressOnSegment >= 1.0f) {
                    train.progressOnSegment = 1.0f;
                    position.coordinates = endPos;
                    if (train.state != TrainState::DECELERATING) {
                        train.state = TrainState::DECELERATING;
                    }
                } else {
                    position.coordinates = startPos + segmentVector * train.progressOnSegment;
                }
            } else {
                // Zero length segment, just snap to end
                position.coordinates = endPos;
                train.progressOnSegment = 1.0;
                if (train.state != TrainState::DECELERATING) {
                    train.state = TrainState::DECELERATING;
                }
            }
        }
    }
}