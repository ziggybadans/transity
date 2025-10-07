#include "TrainMovementSystem.h"
#include "components/GameLogicComponents.h"
#include "Logger.h"
#include "Constants.h"
#include <cmath>
#include <numeric>

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
    const float timeStep = dt.asSeconds();

    // Clear all AtStationComponent instances from the previous frame
    auto atStationView = _registry.view<AtStationComponent>();
    _registry.remove<AtStationComponent>(atStationView.begin(), atStationView.end());

    auto view = _registry.view<TrainTag, TrainMovementComponent, TrainPhysicsComponent, PositionComponent>();

    for (auto entity : view) {
        auto &movement = view.get<TrainMovementComponent>(entity);
        auto &physics = view.get<TrainPhysicsComponent>(entity);
        auto &position = view.get<PositionComponent>(entity);

        if (!_registry.valid(movement.assignedLine)) continue;
        const auto &line = _registry.get<LineComponent>(movement.assignedLine);
        if (line.curvePoints.size() < 2) continue;

        bool justStopped = false;

        // Sort stop distances for finding the next stop
        std::vector<float> stopDistances;
        stopDistances.reserve(line.stops.size());
        for (const auto& stopInfo : line.stops) {
            stopDistances.push_back(stopInfo.distanceAlongCurve);
        }
        std::sort(stopDistances.begin(), stopDistances.end());

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
        }

        float distanceToTravel = physics.currentSpeed * timeStep;
        float nextDistance = movement.distanceAlongCurve + (movement.direction == TrainDirection::FORWARD ? distanceToTravel : -distanceToTravel);

        float nextStopDistance = -1.0f;
        if (movement.direction == TrainDirection::FORWARD) {
            for (float stopDist : stopDistances) {
                if (stopDist > movement.distanceAlongCurve) {
                    nextStopDistance = stopDist;
                    break;
                }
            }
        } else { // BACKWARD
            for (size_t i = stopDistances.size(); i > 0; --i) {
                if (stopDistances[i-1] < movement.distanceAlongCurve) {
                    nextStopDistance = stopDistances[i-1];
                    break;
                }
            }
        }

        if (nextStopDistance != -1.0f) {
            float decelerationDistance = (physics.currentSpeed * physics.currentSpeed) / (2.0f * physics.acceleration);
            if (std::abs(nextStopDistance - movement.distanceAlongCurve) <= decelerationDistance) {
                movement.state = TrainState::DECELERATING;
            }
        }
        
        if (movement.state == TrainState::DECELERATING) {
            physics.currentSpeed -= physics.acceleration * timeStep;
            if (physics.currentSpeed < 0) physics.currentSpeed = 0;

            float distanceToTravelThisFrame = physics.currentSpeed * timeStep;
            float distanceToStop = std::abs(nextStopDistance - movement.distanceAlongCurve);

            // If we are going to pass the stop, or if we have stopped moving, then stop.
            if (distanceToTravelThisFrame >= distanceToStop || physics.currentSpeed == 0) {
                movement.distanceAlongCurve = nextStopDistance;
                movement.state = TrainState::STOPPED;
                movement.stopTimer = Constants::TRAIN_STOP_DURATION;
                physics.currentSpeed = 0;
                justStopped = true;
            } else {
                movement.distanceAlongCurve += (movement.direction == TrainDirection::FORWARD ? distanceToTravelThisFrame : -distanceToTravelThisFrame);
            }
        } else if (movement.state == TrainState::MOVING || movement.state == TrainState::ACCELERATING) {
            movement.distanceAlongCurve = nextDistance;
        }

        // If at the end of the line, ensure the train stops properly
        if ((movement.distanceAlongCurve >= line.totalDistance || movement.distanceAlongCurve <= 0.0f) && movement.state != TrainState::STOPPED) {
            movement.distanceAlongCurve = (movement.distanceAlongCurve >= line.totalDistance) ? line.totalDistance : 0.0f;
            movement.state = TrainState::STOPPED;
            movement.stopTimer = Constants::TRAIN_STOP_DURATION;
            physics.currentSpeed = 0;
            justStopped = true;
        }

        if (justStopped) {
            entt::entity stationEntity = entt::null;
            for (const auto& stopInfo : line.stops) {
                if (std::abs(stopInfo.distanceAlongCurve - movement.distanceAlongCurve) < 0.001f) {
                    stationEntity = stopInfo.stationEntity;
                    break;
                }
            }

            if (_registry.valid(stationEntity)) {
                _registry.emplace<AtStationComponent>(entity, stationEntity);
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
}