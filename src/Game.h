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
#include "world/WorldMap.h"
#include "graphics/Renderer.h"
#include "graphics/Camera.h"
#include "utility/ThreadPool.h"
#include "utility/Task.h"
#include "managers/WindowManager.h"
#include "managers/UIManager.h"
#include "managers/ResourceManager.h"
#include "registry/ActionRegistrar.h"

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
    /* Initialization Methods */
    bool InitManagers();
    bool LoadResources();

    /* Game Loop Methods */
    void ProcessEvents();
    void UpdateNonSimulation(float dt);
    void Render();

    /* Managers */
    InitializationManager m_initManager;
    std::shared_ptr<EventManager> m_eventManager;
    std::shared_ptr<WindowManager> m_windowManager;
    std::shared_ptr<UIManager> m_uiManager;

    /* Core Systems */
    std::unique_ptr<ThreadPool> m_threadPool;
    std::unique_ptr<Renderer> m_renderer;
    std::shared_ptr<Camera> m_camera;
    std::shared_ptr<InputManager> m_inputManager;

    /* Action Registrar */
    std::unique_ptr<ActionRegistrar> m_actionRegistrar;

    /* Game State */
    std::shared_ptr<WorldMap> m_worldMap;
    mutable std::mutex m_worldMapMutex;
    std::atomic<bool> m_isRunning;
    std::atomic<float> m_timeScale;

    /* Window Settings */
    sf::VideoMode m_videoMode;
    std::string m_windowTitle;
    sf::Clock m_deltaClock;

    std::shared_ptr<ResourceManager> m_resourceManager;
};
