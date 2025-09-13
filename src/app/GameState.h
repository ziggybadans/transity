#pragma once

#include "InteractionMode.h"

enum class AppState { LOADING, RUNNING };

struct GameState {
    AppState currentAppState = AppState::LOADING;
    InteractionMode currentInteractionMode = InteractionMode::SELECT;
};
