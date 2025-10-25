#pragma once

#include "app/InteractionMode.h"
#include <SFML/System/Time.hpp>
#include <entt/entt.hpp>
#include <optional>
#include <string>

enum class AppState { LOADING = 0, PLAYING = 1, QUITTING = 2, MAIN_MENU = 3, PAUSED = 4 };

enum class WorldType { PROCEDURAL = 0, REAL = 1 };

enum class GameMode { CAREER = 0, SANDBOX = 1 };

struct GameState {
    InteractionMode currentInteractionMode = InteractionMode::SELECT;
    AppState currentAppState = AppState::MAIN_MENU;
    std::optional<entt::entity> selectedEntity;
    std::optional<entt::entity> passengerOriginStation;
    float timeMultiplier = 1.0f;
    float preEditTimeMultiplier = 1.0f;
    sf::Time totalElapsedTime;
    bool elevationChecksEnabled = true;
    std::string worldName = "New World";
    WorldType worldType = WorldType::PROCEDURAL;
    GameMode gameMode = GameMode::CAREER;
};
