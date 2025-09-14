#pragma once

#include "InteractionMode.h"

enum class AppState { LOADING, PLAYING, QUITTING };

struct GameState {
    AppState currentAppState = AppState::LOADING;
    InteractionMode currentInteractionMode = InteractionMode::SELECT;
};