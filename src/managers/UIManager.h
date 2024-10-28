#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <memory>

#include "InitializationManager.h"
#include "../world/WorldMap.h"

class UIManager : public IInitializable {
public:
    UIManager(std::shared_ptr<WorldMap> worldMap);
    ~UIManager();

    bool Init() override;

    void SetWindow(sf::RenderWindow& window);

    void ProcessEvent(const sf::Event& event);

    void Update(float deltaTime);

    void Render();

    void Shutdown();

private:
    bool initialized;
    sf::RenderWindow* renderWindow;
    std::shared_ptr<WorldMap> worldMap;

    // For storing color and thickness values
    float thickness;
    float color[4]; // RGBA values between 0.0f and 1.0f
};
