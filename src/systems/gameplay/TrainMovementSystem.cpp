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

        if (!_registry.valid(train.assignedLine)) continue;
        const auto &line = _registry.get<LineComponent>(train.assignedLine);
        if (line.stops.size() < 2) continue;

        // Handle stopped state
        if (train.state == TrainState::STOPPED) {
            train.stopTimer -= timeStep;
            if (train.stopTimer <= 0.0f) {
                train.state = TrainState::ACCELERATING;
                train.isApproachingStation = false; // Reset approach flag on departure
            }
            continue;
        }

        // If approaching a station on a curve, handle it separately
        if (train.isApproachingStation) {
            train.currentSpeed -= train.acceleration * timeStep;
            train.decelerationProgress += train.currentSpeed * timeStep;
            
            float t = train.decelerationProgress / train.decelerationDistance;
            if (t > 1.0f) t = 1.0f;

            // Quadratic Bezier curve interpolation
            sf::Vector2f p0 = train.approachCurveStart;
            sf::Vector2f p1 = train.approachCurveControl;
            sf::Vector2f p2 = _registry.get<PositionComponent>(line.stops[train.currentSegmentIndex]).coordinates; // The actual station center

            float one_minus_t = 1.0f - t;
            position.coordinates = one_minus_t * one_minus_t * p0 + 2.0f * one_minus_t * t * p1 + t * t * p2;

            if (train.currentSpeed <= 0.0f || t >= 1.0f) {
                train.currentSpeed = 0.0f;
                train.state = TrainState::STOPPED;
                train.stopTimer = TrainComponent::STOP_DURATION;
                position.coordinates = p2; // Snap to final station position
                train.progressOnSegment = 0.0f;
                train.isApproachingStation = false;

                // Reverse direction at terminal stations
                if (train.direction == TrainDirection::FORWARD && train.currentSegmentIndex >= line.stops.size() - 1) {
                    train.direction = TrainDirection::BACKWARD;
                } else if (train.direction == TrainDirection::BACKWARD && train.currentSegmentIndex <= 0) {
                    train.direction = TrainDirection::FORWARD;
                }
            }
            continue; // Skip normal movement logic
        }

        // --- Normal Movement Logic ---
        int nextStopIndex;
        int segmentIndexForOffset = -1;
        if (train.direction == TrainDirection::FORWARD) {
            nextStopIndex = train.currentSegmentIndex + 1;
            segmentIndexForOffset = train.currentSegmentIndex;
        } else { // BACKWARD
            nextStopIndex = train.currentSegmentIndex - 1;
            segmentIndexForOffset = train.currentSegmentIndex - 1;
        }

        // Determine segment start and end points
        sf::Vector2f startPos;
        if (train.state == TrainState::ACCELERATING) { // Just left a station
            startPos = _registry.get<PositionComponent>(line.stops[train.currentSegmentIndex]).coordinates;
        } else { // In transit
            sf::Vector2f offset(0.f, 0.f);
            int prevSegmentIndex = (train.direction == TrainDirection::FORWARD) ? train.currentSegmentIndex -1 : train.currentSegmentIndex;
            if (prevSegmentIndex >= 0 && prevSegmentIndex < line.pathOffsets.size()) {
                 offset = line.pathOffsets[prevSegmentIndex];
            }
            startPos = _registry.get<PositionComponent>(line.stops[train.currentSegmentIndex]).coordinates + offset;
        }

        sf::Vector2f endPos;
        sf::Vector2f segmentOffset(0.f, 0.f);
        if (segmentIndexForOffset >= 0 && segmentIndexForOffset < line.pathOffsets.size()) {
            segmentOffset = line.pathOffsets[segmentIndexForOffset];
        }
        if (nextStopIndex >= 0 && nextStopIndex < line.stops.size()) {
            endPos = _registry.get<PositionComponent>(line.stops[nextStopIndex]).coordinates + segmentOffset;
        } else { // Approaching a terminal
            endPos = _registry.get<PositionComponent>(line.stops[train.currentSegmentIndex]).coordinates + segmentOffset;
        }

        sf::Vector2f segmentVector = endPos - startPos;
        float segmentLength = std::sqrt(segmentVector.x * segmentVector.x + segmentVector.y * segmentVector.y);
        float distanceToNextStop = (1.0f - train.progressOnSegment) * segmentLength;

        // --- State Transitions ---
        if (train.state == TrainState::ACCELERATING) {
            train.currentSpeed += train.acceleration * timeStep;
            if (train.currentSpeed >= train.maxSpeed) {
                train.currentSpeed = train.maxSpeed;
                train.state = TrainState::MOVING;
            }
        }

        float decelerationDistance = (train.currentSpeed * train.currentSpeed) / (2.0f * train.acceleration);
        if (distanceToNextStop <= decelerationDistance && segmentLength > 0 && nextStopIndex >= 0 && nextStopIndex < line.stops.size()) {
            train.isApproachingStation = true;
            train.decelerationDistance = distanceToNextStop;
            train.decelerationProgress = 0.0f;
            train.approachCurveStart = position.coordinates;
            train.approachCurveControl = endPos; // Use the offset endpoint as the control point
            train.currentSegmentIndex = nextStopIndex; // Target the next station
            train.state = TrainState::DECELERATING;
            continue; // Switch to approach logic next frame
        }

        // --- Position Update ---
        if (segmentLength > 0.0f) {
            float distanceToTravel = train.currentSpeed * timeStep;
            train.progressOnSegment += distanceToTravel / segmentLength;

            if (train.progressOnSegment >= 1.0f) {
                train.progressOnSegment = 0.0f;
                train.currentSegmentIndex = nextStopIndex;
                position.coordinates = endPos;
            } else {
                position.coordinates = startPos + segmentVector * train.progressOnSegment;
            }
        } else if (nextStopIndex >= 0 && nextStopIndex < line.stops.size()) {
            train.progressOnSegment = 0.0f;
            train.currentSegmentIndex = nextStopIndex;
        }
    }
}