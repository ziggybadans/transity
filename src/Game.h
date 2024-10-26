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

// Forward declarations
class WindowManager;
class UIManager;

class Game : public IInitializable {
public:
    Game();
    ~Game();

    // Initialize the game
    bool Init() override;

    // Run the main game loop
    void Run();

    // Shutdown the game and clean up resources
    void Shutdown();

private:
    // Managers
    InitializationManager initManager;
    std::shared_ptr<EventManager> eventManager;
    std::shared_ptr<WindowManager> windowManager;
    std::shared_ptr<UIManager> uiManager;

    // Modules
    std::unique_ptr<ThreadPool> threadPool;
    std::unique_ptr<Renderer> renderer;
    std::shared_ptr<Camera> camera;
    std::unique_ptr<InputManager> inputManager;

    std::shared_ptr<WorldMap> worldMap;
    mutable std::mutex worldMapMutex;

    // Game state
    std::atomic<bool> isRunning;

    // Video settings
    sf::VideoMode videoMode;
    std::string windowTitle;

    sf::Clock deltaClock;

    // Initialization helpers
    bool InitManagers();
    bool LoadResources();

    // Main loop functions
    void ProcessEvents();
    void Update(float dt);
    void Render();
};
