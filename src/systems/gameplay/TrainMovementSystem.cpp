#include "TrainMovementSystem.h"
#include "components/TrainComponents.h"
#include "components/LineComponents.h"
#include "components/GameLogicComponents.h"
#include "Logger.h"
#include "Constants.h"
#include <cmath>

TrainMovementSystem::TrainMovementSystem(entt::registry& registry)
    : _registry(registry) {
    LOG_DEBUG("TrainMovementSystem", "TrainMovementSystem created.");
}

sf::Vector2f TrainMovementSystem::getPositionAtDistance(const LineComponent& line, float distance) {
    if (line.curvePoints.empty()) {
        return sf::Vector2f();
    }
    if (distance <= 0.0f) {
        return line.curvePoints.front();
    }
    if (distance >= line.totalDistance) {
        return line.curvePoints.back();
    }

    float currentDistance = 0.0f;
    for (size_t i = 0; i < line.curvePoints.size() - 1; ++i) {
        const auto& p1 = line.curvePoints[i];
        const auto& p2 = line.curvePoints[i + 1];
        float segmentLength = std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
        
        if (currentDistance + segmentLength >= distance) {
            float remainingDistance = distance - currentDistance;
            float t = remainingDistance / segmentLength;
            return p1 + (p2 - p1) * t;
        }
        currentDistance += segmentLength;
    }

    return line.curvePoints.back();
}

void TrainMovementSystem::update(sf::Time dt) {
    // Clear all AtStationComponent instances from the previous frame
    auto atStationView = _registry.view<AtStationComponent>();
    _registry.remove<AtStationComponent>(atStationView.begin(), atStationView.end());

    auto view = _registry.view<TrainTag, TrainMovementComponent, TrainPhysicsComponent, PositionComponent>();
    for (auto entity : view) {
        if (!_registry.valid(view.get<TrainMovementComponent>(entity).assignedLine)) continue;
        
        const auto &line = _registry.get<LineComponent>(view.get<TrainMovementComponent>(entity).assignedLine);
        if (line.curvePoints.size() < 2) continue;

        auto &movement = view.get<TrainMovementComponent>(entity);
        auto &physics = view.get<TrainPhysicsComponent>(entity);

        updateTrainStateAndSpeed(movement, physics, line, dt);
        updateTrainPositionAndStop(entity, dt);
    }
}

std::optional<float> TrainMovementSystem::findNextStopDistance(const TrainMovementComponent& movement, const LineComponent& line) {
    std::vector<float> stopDistances;
    stopDistances.reserve(line.stops.size());
    for (const auto& stopInfo : line.stops) {
        stopDistances.push_back(stopInfo.distanceAlongCurve);
    }
    std::sort(stopDistances.begin(), stopDistances.end());

    if (movement.direction == TrainDirection::FORWARD) {
        for (float stopDist : stopDistances) {
            if (stopDist > movement.distanceAlongCurve) {
                return stopDist;
            }
        }
    } else { // BACKWARD
        for (size_t i = stopDistances.size(); i > 0; --i) {
            if (stopDistances[i-1] < movement.distanceAlongCurve) {
                return stopDistances[i-1];
            }
        }
    }
    return std::nullopt;
}

void TrainMovementSystem::updateTrainStateAndSpeed(TrainMovementComponent& movement, TrainPhysicsComponent& physics, const LineComponent& line, sf::Time dt) {
    const float timeStep = dt.asSeconds();

    if (movement.state == TrainState::STOPPED) {
        movement.stopTimer -= timeStep;
        if (movement.stopTimer <= 0.0f) {
            const float epsilon = 0.001f;
            if (movement.direction == TrainDirection::FORWARD && movement.distanceAlongCurve >= line.totalDistance - epsilon) {
                movement.direction = TrainDirection::BACKWARD;
            } else if (movement.direction == TrainDirection::BACKWARD && movement.distanceAlongCurve <= epsilon) {
                movement.direction = TrainDirection::FORWARD;
            }
            movement.state = TrainState::ACCELERATING;
        }
    }

    if (movement.state == TrainState::ACCELERATING) {
        physics.currentSpeed += physics.acceleration * timeStep;
        if (physics.currentSpeed >= physics.maxSpeed) {
            physics.currentSpeed = physics.maxSpeed;
            movement.state = TrainState::MOVING;
        }
    } else if (movement.state == TrainState::DECELERATING) {
        physics.currentSpeed -= physics.acceleration * timeStep;
        if (physics.currentSpeed < 0) physics.currentSpeed = 0;
    }

    if (movement.state == TrainState::MOVING || movement.state == TrainState::ACCELERATING) {
        if (auto nextStopDist = findNextStopDistance(movement, line)) {
            float decelerationDistance = (physics.currentSpeed * physics.currentSpeed) / (2.0f * physics.acceleration);
            if (std::abs(nextStopDist.value() - movement.distanceAlongCurve) <= decelerationDistance) {
                movement.state = TrainState::DECELERATING;
            }
        }
    }
}

void TrainMovementSystem::updateTrainPositionAndStop(entt::entity trainEntity, sf::Time dt) {
    auto &movement = _registry.get<TrainMovementComponent>(trainEntity);
    auto &physics = _registry.get<TrainPhysicsComponent>(trainEntity);
    auto &position = _registry.get<PositionComponent>(trainEntity);
    const auto &line = _registry.get<LineComponent>(movement.assignedLine);
    
    const float timeStep = dt.asSeconds();
    bool justStopped = false;

    float distanceToTravel = physics.currentSpeed * timeStep;

    if (movement.state == TrainState::DECELERATING) {
        if (auto nextStopDist = findNextStopDistance(movement, line)) {
            float distanceToStop = std::abs(nextStopDist.value() - movement.distanceAlongCurve);
            if (distanceToTravel >= distanceToStop || physics.currentSpeed == 0) {
                movement.distanceAlongCurve = nextStopDist.value();
                justStopped = true;
            }
        }
    }
    
    if (!justStopped && movement.state != TrainState::STOPPED) {
        movement.distanceAlongCurve += (movement.direction == TrainDirection::FORWARD ? distanceToTravel : -distanceToTravel);
    }

    if ((movement.distanceAlongCurve >= line.totalDistance || movement.distanceAlongCurve <= 0.0f) && movement.state != TrainState::STOPPED) {
        movement.distanceAlongCurve = (movement.distanceAlongCurve >= line.totalDistance) ? line.totalDistance : 0.0f;
        justStopped = true;
    }

    if (justStopped) {
        movement.state = TrainState::STOPPED;
        movement.stopTimer = Constants::TRAIN_STOP_DURATION;
        physics.currentSpeed = 0;

        entt::entity stationEntity = entt::null;
        for (const auto& stopInfo : line.stops) {
            if (std::abs(stopInfo.distanceAlongCurve - movement.distanceAlongCurve) < 0.001f) {
                stationEntity = stopInfo.stationEntity;
                break;
            }
        }
        if (_registry.valid(stationEntity)) {
            _registry.emplace<AtStationComponent>(trainEntity, stationEntity);
            LOG_TRACE("TrainMovementSystem", "Train arrived at station and AtStationComponent added.");
        }
    }

    position.coordinates = getPositionAtDistance(line, movement.distanceAlongCurve);
    
    // Apply path offsets
    size_t segmentIndex = 0;
    float tempDist = 0.f;
    for(size_t i = 0; i < line.curvePoints.size() - 1; ++i) {
        const auto& p1 = line.curvePoints[i];
        const auto& p2 = line.curvePoints[i+1];
        float segLen = std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
        if (tempDist + segLen >= movement.distanceAlongCurve) {
            segmentIndex = line.curveSegmentIndices[i];
            break;
        }
        tempDist += segLen;
    }

    if (segmentIndex < line.pathOffsets.size()) {
        position.coordinates += line.pathOffsets[segmentIndex];
    }
}