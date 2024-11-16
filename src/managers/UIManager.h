#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include <functional>
#include <atomic>
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

    // Updates the UI elements each frame.
    void Update(float deltaTime);

    // Renders the ImGui elements to the screen.
    void Render();

    // Shuts down ImGui and cleans up resources.
    void Shutdown();

    // Register callbacks for UI actions
    void RegisterUIAction(const std::string& action, std::function<void()> callback);

    // Setter for timeScale pointer
    void SetTimeScalePointer(std::atomic<float>* ptr);

private:
    bool initialized;
    sf::RenderWindow* renderWindow;
    std::shared_ptr<WorldMap> worldMap;

    std::atomic<float>* timeScalePtr;

    // UI action callbacks
    std::vector<std::pair<std::string, std::function<void()>>> uiActionCallbacks;

    // UI callback methods
    void EmitUIAction(const std::string& action);

    // Helper methods for UI panels
    void RenderLineProperties();
    void RenderStationProperties();
};
