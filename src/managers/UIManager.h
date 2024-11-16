#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include "EventManager.h"
#include "../world/WorldMap.h"

class UIManager {
public:
    UIManager(std::shared_ptr<WorldMap> worldMap);
    ~UIManager();

    /* Core UI Methods */
    bool Init();
    void ProcessEvent(const sf::Event& event);
    void Update(float deltaTime);
    void Render();
    void Shutdown();

    /* Setters */
    void SetWindow(sf::RenderWindow& window);

private:
    /* UI State */
    bool m_initialized;
    sf::RenderWindow* m_renderWindow;
    std::shared_ptr<WorldMap> m_worldMap;
    float* m_timeScalePtr;

    // New member variable to store FPS
    float m_fps;
};
