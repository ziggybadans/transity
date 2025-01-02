#pragma once

#include <imgui.h>
#include <imgui-SFML.h>
#include <SFML/Graphics.hpp>
#include <memory>
#include "../settings/GameSettings.h"
#include "../interfaces/IInitializable.h"
#include "../managers/WindowManager.h"
#include "../managers/InputManager.h"
#include "../core/StateManager.h"

class UIManager : public IInitializable {
public:
    UIManager();
    ~UIManager();

    /* Core UI Methods */
    bool Init() override;
    void ProcessEvent(const sf::Event& event);
    void Update(float deltaTime);
    void Render();
    void Shutdown();

    /* Setters */
    void SetWindow(sf::RenderWindow& window);
    void SetGameSettings(std::shared_ptr<GameSettings> settings) { m_gameSettings = settings; }
    void SetWindowManager(std::shared_ptr<WindowManager> windowManager) { m_windowManager = windowManager; }
    void SetInputManager(std::shared_ptr<InputManager> inputManager) { m_inputManager = inputManager; }
    void SetStateManager(std::shared_ptr<StateManager> stateManager) { m_stateManager = stateManager; }
    void SetMap(std::shared_ptr<Map> map) { m_map = map; }

private:
    /* UI Panels */
    void RenderPerformanceOverlay();
    void RenderSettingsPanel();
    void RenderVideoSettings();
    void RenderGameplaySettings();
    void RenderPerformanceWindow();
    void RenderInfoPanel();
    void RenderGUI();

    /* UI State */
    bool m_initialized;
    sf::RenderWindow* m_renderWindow;
    float* m_timeScalePtr;
    float m_fps;
    bool m_showSettingsPanel;
    bool m_showPerformanceWindow;

    /* Game Settings */
    std::shared_ptr<GameSettings> m_gameSettings;
    std::shared_ptr<WindowManager> m_windowManager;
    std::shared_ptr<InputManager> m_inputManager;
    std::shared_ptr<StateManager> m_stateManager;
    std::shared_ptr<Map> m_map;
};
