#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <memory>

#include "InitializationManager.h"

class UIManager : public IInitializable { // Changed to public inheritance
public:
    UIManager();
    ~UIManager();

    // Initialize ImGui without parameters
    bool Init() override;

    // Set the RenderWindow before initialization
    void SetWindow(sf::RenderWindow& window);

    // Process SFML events for ImGui
    void ProcessEvent(const sf::Event& event);

    // Update ImGui with delta time
    void Update(float deltaTime);

    // Render ImGui
    void Render();

    // Shutdown ImGui
    void Shutdown();

private:
    bool initialized;
    sf::RenderWindow* renderWindow; // Pointer to the SFML window
};
