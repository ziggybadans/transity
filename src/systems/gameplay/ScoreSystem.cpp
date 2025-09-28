#include "ScoreSystem.h"
#include "components/GameLogicComponents.h"
#include "components/PassengerComponents.h"
#include "Logger.h"

ScoreSystem::ScoreSystem(entt::registry& registry)
    : _registry(registry) {
    // Create a persistent entity to hold the score
    _scoreEntity = _registry.create();
    _registry.emplace<GameScoreComponent>(_scoreEntity);
    LOG_DEBUG("ScoreSystem", "ScoreSystem created and score entity initialized.");
}

void ScoreSystem::update(sf::Time dt) {
    auto passengerView = _registry.view<PassengerComponent>();
    int score = 0;
    for (auto entity : passengerView) {
        const auto& passenger = passengerView.get<PassengerComponent>(entity);
        if (passenger.state == PassengerState::ON_TRAIN) {
            score++;
        }
    }

    auto& scoreComponent = _registry.get<GameScoreComponent>(_scoreEntity);
    scoreComponent.score = score;
}