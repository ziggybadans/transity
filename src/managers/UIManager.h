#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <memory>

#include "InitializationManager.h"
#include "../world/WorldMap.h"

class UIManager : public IInitializable {
public:
    // Constructor that initializes the UIManager with a shared pointer to the WorldMap.
    UIManager(std::shared_ptr<WorldMap> worldMap);
    ~UIManager();

    // Initializes ImGui and other UI components.
    bool Init() override;

    // Sets the reference to the SFML RenderWindow.
    void SetWindow(sf::RenderWindow& window);

    // Processes input events from the SFML window (e.g., mouse clicks, keyboard inputs).
    void ProcessEvent(const sf::Event& event);

    // Updates the UI elements each frame.
    void Update(float deltaTime);

    // Renders the ImGui elements to the screen.
    void Render();

    // Shuts down ImGui and cleans up resources.
    void Shutdown();

    // Setter for timeScale pointer
    void SetTimeScalePointer(std::atomic<float>* ptr);

private:
    bool initialized; // Flag to indicate if the UIManager has been initialized.
    sf::RenderWindow* renderWindow; // Pointer to the SFML RenderWindow.
    std::shared_ptr<WorldMap> worldMap; // Reference to the world map for accessing game data.

    // Time control pointer
    std::atomic<float>* timeScalePtr;

    // Parameters for UI customization.
    float thickness; // Thickness value for rendering elements.
    float color[4]; // RGBA values for setting color (values between 0.0f and 1.0f).

    float speedKmPerHour; // Speed for the selected line
};
