#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "../input/InteractionMode.h"
#include "../event/LineEvents.h"

class UI {
public:
    UI(sf::RenderWindow& window);
    ~UI();
    void initialize();
    void processEvent(const sf::Event& event);
    void update(sf::Time deltaTime, size_t numStationsInActiveLine);
    void renderFrame();
    void cleanupResources();
    InteractionMode getInteractionMode() const;

    const std::vector<FinalizeLineEvent>& getUiEvents() const;
    void clearUiEvents();

private:
    sf::RenderWindow& m_window;
    InteractionMode m_currentInteractionMode;
    std::vector<FinalizeLineEvent> m_uiEvents;
};