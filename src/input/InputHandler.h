#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <entt/entt.hpp>
#include "../core/Camera.h"
#include "../input/InteractionMode.h"
#include "../event/EventBus.h"
#include "../event/InputEvents.h"
#include "../event/LineEvents.h"

class InputHandler {
public:
    // Constructor now takes the event bus
    InputHandler(EventBus& eventBus);

    // handleGameEvent now takes a const Camera&
    void handleGameEvent(const sf::Event& event, InteractionMode currentMode, const Camera& camera, sf::RenderWindow& window, entt::registry& registry);
    void update(sf::Time dt, const Camera& camera);

    // All command and game event-related methods are removed

private:
    EventBus& _eventBus; // Reference to the event bus

    // Constants for zoom calculation
    float _zoomFactor;
    float _unzoomFactor;
};
