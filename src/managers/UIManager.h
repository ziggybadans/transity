#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include "EventManager.h"
#include "../world/WorldMap.h"

class UIManager {
public:
    // Constructor that initializes the UIManager with a shared pointer to the WorldMap.
    UIManager(std::shared_ptr<WorldMap> worldMap);
    ~UIManager();

    // Initialize ImGui and other UI components.
    bool Init();

    // Sets the reference to the SFML RenderWindow.
    void SetWindow(sf::RenderWindow& window);

    // Processes input events from the SFML window (e.g., mouse clicks, keyboard inputs).
    void ProcessEvent(const sf::Event& event);

    // Updates UI elements every frame.
    void Update(float deltaTime);

    // Renders the UI elements.
    void Render();

    // Shutdown and clean up UI resources.
    void Shutdown();

private:
    bool initialized;
    sf::RenderWindow* renderWindow;
    std::shared_ptr<WorldMap> worldMap;
    float* timeScalePtr;

    // UI rendering methods
    void RenderPlaceAreaInfo();
};
