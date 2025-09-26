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

std::vector<float> TrainMovementSystem::getStopDistances(const LineComponent& line) {
    std::vector<float> stopDistances;
    if (line.points.empty()) {
        return stopDistances;
    }

    std::vector<float> pointDistances(line.points.size(), 0.0f);
    float accumulatedDistance = 0.0f;

    // This is an approximation. For perfect accuracy, we'd need to map the original points to the curve.
    // However, since the curve passes through the points, we can map them by finding the closest curve point.
    // For simplicity, we'll assume the original points are on the curve.
    // A more robust solution would be to pre-calculate the distances of the original points along the curve.
    
    float currentCurveDistance = 0.0f;
    size_t nextPointIndex = 0;
    for(size_t i = 0; i < line.curvePoints.size(); ++i) {
        if (nextPointIndex < line.points.size()) {
            const auto& p = line.points[nextPointIndex];
            if (line.curvePoints[i] == p.position) {
                pointDistances[nextPointIndex] = currentCurveDistance;
                if (p.type == LinePointType::STOP) {
                    stopDistances.push_back(currentCurveDistance);
                }
                nextPointIndex++;
            }
        }
        if (i < line.curvePoints.size() - 1) {
            const auto& p1 = line.curvePoints[i];
            const auto& p2 = line.curvePoints[i + 1];
            currentCurveDistance += std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2));
        }
    }

    return stopDistances;
}


void TrainMovementSystem::update(sf::Time dt) {
    const float timeStep = dt.asSeconds();
    auto view = _registry.view<TrainTag, TrainMovementComponent, TrainPhysicsComponent, PositionComponent>();

    for (auto entity : view) {
        auto &movement = view.get<TrainMovementComponent>(entity);
        auto &physics = view.get<TrainPhysicsComponent>(entity);
        auto &position = view.get<PositionComponent>(entity);

        if (!_registry.valid(movement.assignedLine)) continue;
        const auto &line = _registry.get<LineComponent>(movement.assignedLine);
        if (line.curvePoints.size() < 2) continue;

        auto stopDistances = getStopDistances(line);

        if (movement.state == TrainState::STOPPED) {
            movement.stopTimer -= timeStep;
            if (movement.stopTimer <= 0.0f) {
                if (movement.direction == TrainDirection::FORWARD && movement.distanceAlongCurve >= line.totalDistance) {
                    movement.direction = TrainDirection::BACKWARD;
                } else if (movement.direction == TrainDirection::BACKWARD && movement.distanceAlongCurve <= 0.0f) {
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

             if (std::abs(nextDistance - nextStopDistance) < std::abs(movement.distanceAlongCurve - nextStopDistance)) {
                 movement.distanceAlongCurve = nextDistance;
             } else {
                 movement.distanceAlongCurve = nextStopDistance;
                 movement.state = TrainState::STOPPED;
                 movement.stopTimer = Constants::TRAIN_STOP_DURATION;
                 physics.currentSpeed = 0;
             }
        }

        if (movement.state == TrainState::MOVING || movement.state == TrainState::ACCELERATING) {
            movement.distanceAlongCurve = nextDistance;
        }

        if (movement.distanceAlongCurve > line.totalDistance) {
            movement.distanceAlongCurve = line.totalDistance;
        } else if (movement.distanceAlongCurve < 0) {
            movement.distanceAlongCurve = 0;
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