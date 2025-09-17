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
    auto view = _registry.view<TrainComponent, PositionComponent>();

    for (auto entity : view) {
        auto &train = view.get<TrainComponent>(entity);
        auto &position = view.get<PositionComponent>(entity);

        if (train.state == TrainState::STOPPED) {
            train.stopTimer -= dt.asSeconds();
            if (train.stopTimer <= 0.0f) {
                train.state = TrainState::MOVING;
            }
            continue;
        }

        if (!_registry.valid(train.assignedLine)) {
            continue;
        }

        const auto &line = _registry.get<LineComponent>(train.assignedLine);
        if (line.stops.size() < 2) {
            continue;
        }

        float distanceToTravel = train.speed * dt.asSeconds();

        while (distanceToTravel > 0.0f) {
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

            if (segmentLength <= 0.0f) {
                train.progressOnSegment = 1.0f;
            } else {
                float progressThisFrame = distanceToTravel / segmentLength;
                train.progressOnSegment += progressThisFrame;
            }

            if (train.progressOnSegment >= 1.0f) {
                float overshootProgress = train.progressOnSegment - 1.0f;
                distanceToTravel = overshootProgress * segmentLength;
                
                position.coordinates = nextStopPos;
                train.currentSegmentIndex = nextStopIndex;
                train.progressOnSegment = 0.0f;

                train.state = TrainState::STOPPED;
                train.stopTimer = TrainComponent::STOP_DURATION;
                
            } else {
                position.coordinates = currentStopPos + segmentVector * train.progressOnSegment;
                distanceToTravel = 0.0f;
            }
        }
    }
}