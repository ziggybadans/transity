// In src/event/InputEvents.h

#pragma once

#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>

// General Events
struct WindowCloseEvent {};

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

// Line Management Events (These already exist but are good to consolidate conceptually)
// We will continue to use the existing LineEvents.h for these for now.
// #include "LineEvents.h"
