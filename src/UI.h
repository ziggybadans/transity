#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "InteractionMode.h"

class UI {
public:
    UI(sf::RenderWindow& window);
    ~UI();
    void init();
    void processEvent(const sf::Event& event);
    void update(sf::Time deltaTime);
    void render();
    void cleanup();
    InteractionMode getInteractionMode() const;

private:
    sf::RenderWindow& m_window;
    InteractionMode m_currentInteractionMode;
};