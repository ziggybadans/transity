#pragma once

#include "event/EventBus.h"
#include "event/UIEvents.h"
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

class PerformanceMonitor;
class Camera;
class GameState;
class ColorManager;

class DebugUI {
public:
    DebugUI(PerformanceMonitor& performanceMonitor, Camera& camera, GameState& gameState, ColorManager& colorManager, EventBus& eventBus, sf::RenderWindow& window);
    ~DebugUI();

    void draw(sf::Time deltaTime);

private:
    void onThemeChanged(const ThemeChangedEvent& event);

    void drawProfilingWindow(sf::Time deltaTime);
    void drawTimeControlWindow();
    void drawSettingsWindow();

    PerformanceMonitor& _performanceMonitor;
    Camera& _camera;
    GameState& _gameState;
    ColorManager& _colorManager;
    sf::RenderWindow& _window;

    entt::scoped_connection _themeChangedConnection;
};