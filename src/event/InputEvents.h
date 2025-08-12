// In src/event/InputEvents.h

#pragma once

#include "../input/InteractionMode.h"
#include "../world/WorldData.h"
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>

// General Events
struct WindowCloseEvent {};

struct MouseButtonPressedEvent {
    sf::Mouse::Button button;
    sf::Vector2i pixelPosition;
    sf::Vector2f worldPosition;
};

// Camera Control Events
struct CameraZoomEvent {
    float zoomDelta;
    sf::Vector2i mousePixelPosition;
};

struct CameraPanEvent {
    sf::Vector2f panDirection;
};

// Station Management Events
struct TryPlaceStationEvent {
    sf::Vector2f worldPosition;
};

// In a relevant event header file
struct InteractionModeChangeEvent {
    InteractionMode newMode;
};

// In a relevant event header file
struct RegenerateWorldRequestEvent {
    WorldGenParams params;
};

struct ToggleNoiseVisualizationEvent {
    bool show;
};

struct ImmediateRedrawEvent {};

// Line Management Events (These already exist but are good to consolidate conceptually)
// We will continue to use the existing LineEvents.h for these for now.
// #include "LineEvents.h"
