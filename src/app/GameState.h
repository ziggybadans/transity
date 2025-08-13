#pragma once

#include "../input/InteractionMode.h"

struct GameState {
    InteractionMode currentInteractionMode = InteractionMode::SELECT;
};
