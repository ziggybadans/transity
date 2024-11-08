#pragma once

#include <SFML/Graphics.hpp>  // Include SFML graphics for rendering
#include <imgui.h>  // Include ImGui for GUI elements
#include <imgui-SFML.h>  // Include ImGui-SFML binding
#include <atomic>  // Include for atomic operations
#include <memory>  // Include for smart pointers
#include <mutex>  // Include for managing concurrent access to shared data

#include "managers/InitializationManager.h"  // Include initialization manager
#include "managers/EventManager.h"  // Include event manager
#include "managers/InputManager.h"  // Include input manager
#include "world/WorldMap.h"  // Include world map representation
#include "graphics/Renderer.h"  // Include rendering system
#include "graphics/Camera.h"  // Include camera class
#include "utility/ThreadPool.h"  // Include thread pool for managing concurrent tasks
#include "utility/Task.h"  // Include task utility for multi-threading

// Forward declarations
class WindowManager;  // Forward declaration of WindowManager
class UIManager;  // Forward declaration of UIManager

// Game class, inheriting from IInitializable to support initialization pattern
class Game : public IInitializable {
public:
    Game();  // Constructor to initialize the game object
    ~Game();  // Destructor to clean up resources

    // Initialize the game
    bool Init() override;  // Override Init from IInitializable to handle game-specific initialization

    // Run the main game loop
    void Run();  // Function to run the main game loop

    // Shutdown the game and clean up resources
    void Shutdown();  // Clean up resources and shut down the game properly

    // Getter for timeScale (optional)
    float GetTimeScale() const { return timeScale.load(); }

    // Setter for timeScale (optional)
    void SetTimeScale(float scale) { timeScale.store(scale); }

private:
    // Managers
    InitializationManager initManager;  // Handles initialization of different game components
    std::shared_ptr<EventManager> eventManager;  // Manages events such as input and window events
    std::shared_ptr<WindowManager> windowManager;  // Manages window creation and events
    std::shared_ptr<UIManager> uiManager;  // Manages UI elements, likely for the debug GUI

    // Modules
    std::unique_ptr<ThreadPool> threadPool;  // Thread pool for running tasks concurrently
    std::unique_ptr<Renderer> renderer;  // Handles rendering of game elements
    std::shared_ptr<Camera> camera;  // Controls the view of the game, e.g., panning and zooming
    std::unique_ptr<InputManager> inputManager;  // Manages player input (keyboard, mouse, etc.)

    std::shared_ptr<WorldMap> worldMap;  // Represents the game world; contains map data
    mutable std::mutex worldMapMutex;  // Mutex to guard access to worldMap, allowing thread-safe modifications

    // Game state
    std::atomic<bool> isRunning;  // Atomic flag to determine if the game is running, ensures thread-safe access

    // Video settings
    sf::VideoMode videoMode;  // Stores video settings, such as screen resolution
    std::string windowTitle;  // Title of the game window

    sf::Clock deltaClock;  // Clock used to track delta time between frames
    std::atomic<float> timeScale;  // Atomic variable to control time scaling

    // Initialization helpers
    bool InitManagers();  // Helper function to initialize various managers
    bool LoadResources();  // Helper function to load game resources (textures, sounds, etc.)

    // Main loop functions
    void ProcessEvents();  // Process input events (keyboard, mouse, etc.)

    // Separate update functions
    void UpdateNonSimulation(float dt);  // Update input, camera, UI
    void UpdateSimulation(float scaledDt); // Update simulation (e.g., trains)

    void Render();  // Render the game, drawing everything to the screen
};
