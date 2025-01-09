#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <mutex>
#include "Camera.h"
#include "../core/StateManager.h"
#include "../world/Map.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    /* Core Renderer Methods */
    bool Init();
    bool InitWithWindow(sf::RenderWindow& window);
    void Render(sf::RenderWindow& window, const Camera& camera, Map& map, StateManager& stateManager);
    void Shutdown();

private:
    /* Renderer State */
    bool m_isInitialized;
    std::mutex m_renderMutex;

    /* World Rendering */
    void RenderMap(sf::RenderWindow& window, Map& map, const Camera& camera, StateManager& stateManager) const;
    void DrawThickLine(sf::RenderWindow& window, const sf::Vector2f& start, const sf::Vector2f& end, float thickness, const sf::Color& color) const;

    sf::Font m_font;
};
