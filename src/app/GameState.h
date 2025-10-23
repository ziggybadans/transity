#pragma once

#include "app/InteractionMode.h"
#include <SFML/System/Time.hpp>
#include <entt/entt.hpp>
#include <optional>

enum class AppState { LOADING, PLAYING, QUITTING };

struct GameState {
    InteractionMode currentInteractionMode = InteractionMode::SELECT;
    AppState currentAppState = AppState::LOADING;
    std::optional<entt::entity> selectedEntity;
    std::optional<entt::entity> passengerOriginStation;
    float timeMultiplier = 1.0f;
    float preEditTimeMultiplier = 1.0f;
    sf::Time totalElapsedTime;
    bool elevationChecksEnabled = true;
};
