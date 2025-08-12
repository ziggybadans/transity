

#pragma once

#include "../input/InteractionMode.h"
#include "../world/WorldData.h"
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>


struct WindowCloseEvent {};

struct MouseButtonPressedEvent {
    sf::Mouse::Button button;
    sf::Vector2i pixelPosition;
    sf::Vector2f worldPosition;
};


struct CameraZoomEvent {
    float zoomDelta;
    sf::Vector2i mousePixelPosition;
};

struct CameraPanEvent {
    sf::Vector2f panDirection;
};


struct TryPlaceStationEvent {
    sf::Vector2f worldPosition;
};


struct InteractionModeChangeEvent {
    InteractionMode newMode;
};


struct RegenerateWorldRequestEvent {
    WorldGenParams params;
};

struct ToggleNoiseVisualizationEvent {
    bool show;
};

struct ImmediateRedrawEvent {};




