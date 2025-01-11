#pragma once

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <atomic>
#include <memory>
#include <mutex>

#include "core/EventManager.h"
#include "core/InputManager.h"
#include "graphics/Renderer.h"
#include "graphics/Camera.h"
#include "core/ThreadManager.h"
#include "core/Task.h"
#include "ui/WindowManager.h"
#include "ui/UIManager.h"
#include "resource/ResourceManager.h"
#include "modding/PluginManager.h"
#include "settings/GameSettings.h"
#include "resource/SaveManager.h"
#include "core/StateManager.h"
#include "world/Map.h"
#include "Simulation.h"

class IInitializable;

class Game {
public:
    Game();
    ~Game();

    bool Init();

    /* Core Game Methods */
    void Run();
    void Shutdown();

    /* Accessors */
    float GetTimeScale() const { return m_timeScale.load(); }
    void SetTimeScale(float scale) { m_timeScale.store(scale); }

private:
    /* Initialization Methods */
    void InitializeWorld();

    /* Game Loop Methods */
    void ProcessEvents();
    void UpdateNonSimulation(float dt); // Updates things like the camera, UI and inputs
    void Render();

    /* Managers */
    std::shared_ptr<EventManager> m_eventManager;
    std::shared_ptr<WindowManager> m_windowManager;
    std::mutex m_windowManagerMutex;
    std::shared_ptr<UIManager> m_uiManager;
    std::unique_ptr<ThreadManager> m_threadManager;
    std::unique_ptr<Renderer> m_renderer;
    std::shared_ptr<Camera> m_camera;
    std::shared_ptr<InputManager> m_inputManager;
    std::shared_ptr<ResourceManager> m_resourceManager;
    std::unique_ptr<PluginManager> m_pluginManager;
    std::shared_ptr<GameSettings> m_gameSettings;
    std::shared_ptr<SaveManager> m_saveManager;

    /* Core Systems */
    std::shared_ptr<Map> m_map;
    std::unique_ptr<Simulation> m_simulation;

    /* Game State */
    mutable std::mutex m_gameMutex;
    //std::atomic<bool> m_isRunning;
    std::atomic<float> m_timeScale;
    std::shared_ptr<StateManager> m_stateManager;

    /* Window Settings */
    sf::VideoMode m_videoMode;
    std::string m_windowTitle;
    sf::Clock m_deltaClock;

    void RegisterSettings();
};
