#pragma once

#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <imgui-SFML.h>
#include <atomic>
#include <memory>
#include <mutex>

#include "managers/InitializationManager.h"
#include "managers/EventManager.h"
#include "managers/InputManager.h"
#include "graphics/Renderer.h"
#include "graphics/Camera.h"
#include "utility/ThreadManager.h"
#include "utility/Task.h"
#include "managers/WindowManager.h"
#include "managers/UIManager.h"
#include "managers/ResourceManager.h"
#include "registry/ActionRegistrar.h"
#include "modding/PluginManager.h"
#include "settings/GameSettings.h"
#include "managers/SaveManager.h"
#include "core/StateManager.h"

class IInitializable;

class Game : public IInitializable {
public:
    Game();
    ~Game();

    /* Virtual Methods */
    bool Init() override;

    /* Core Game Methods */
    void Run();
    void Shutdown();

    /* Accessors */
    float GetTimeScale() const { return m_timeScale.load(); }
    void SetTimeScale(float scale) { m_timeScale.store(scale); }

private:
    /* Game Loop Methods */
    void ProcessEvents();
    void UpdateNonSimulation(float dt); // Updates things like the camera, UI and inputs
    void Render();

    /* Managers */
    std::shared_ptr<EventManager> m_eventManager;
    std::shared_ptr<WindowManager> m_windowManager;
    std::shared_ptr<UIManager> m_uiManager;

    /* Core Systems */
    std::unique_ptr<ThreadManager> m_threadManager;
    std::unique_ptr<Renderer> m_renderer;
    std::shared_ptr<Camera> m_camera;
    std::shared_ptr<InputManager> m_inputManager;

    /* Action Registrar */
    std::unique_ptr<ActionRegistrar> m_actionRegistrar;

    /* Game State */
    mutable std::mutex m_gameMutex;
    //std::atomic<bool> m_isRunning;
    std::atomic<float> m_timeScale;
    std::unique_ptr<StateManager> m_stateManager;

    /* Window Settings */
    sf::VideoMode m_videoMode;
    std::string m_windowTitle;
    sf::Clock m_deltaClock;

    std::shared_ptr<ResourceManager> m_resourceManager;

    std::unique_ptr<PluginManager> m_pluginManager;

    std::shared_ptr<GameSettings> m_gameSettings;
    std::unique_ptr<SaveManager> m_saveManager;

    void RegisterSettings();
};
