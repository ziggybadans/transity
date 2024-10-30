#include "Game.h"
#include "managers/WindowManager.h"
#include "managers/UIManager.h"
#include "managers/InputManager.h"
#include "utility/ThreadPool.h"
#include "graphics/Renderer.h"
#include "graphics/Camera.h"
#include "world/WorldMap.h"

#include <iostream>
#include <future>
#include <thread>
#include <condition_variable>

#include "Constants.h"

// Constructor initializes video mode, window title, and sets isRunning to false
Game::Game()
    : videoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT),
    windowTitle(Constants::WINDOW_TITLE),
    isRunning(false) {}

// Destructor calls Shutdown to release resources
Game::~Game() {
    Shutdown();
}

// Initialize the game
bool Game::Init() {
    if (!InitManagers()) {  // Initialize managers (WindowManager, etc.)
        std::cerr << "Failed to initialize managers." << std::endl;
        return false;
    }

    // Initialize EventManager for handling system events
    eventManager = std::make_shared<EventManager>();

    // Initialize Camera with the current window size
    sf::Vector2u windowSize = windowManager->GetWindow().getSize();
    camera = std::make_shared<Camera>(windowSize);
    camera->setMinZoomLevel(Constants::CAMERA_MIN_ZOOM);  // Set minimum zoom level
    camera->setMaxZoomLevel(Constants::CAMERA_MAX_ZOOM);  // Set maximum zoom level

    // Initialize ThreadPool with number of hardware threads available
    threadPool = std::make_unique<ThreadPool>(std::thread::hardware_concurrency());
    renderer = std::make_unique<Renderer>();
    if (!renderer->Init(windowManager->GetWindow())) {  // Initialize renderer with the window
        std::cerr << "Failed to initialize Renderer." << std::endl;
        return false;
    }

    // Load initial resources asynchronously
    if (!LoadResources()) {
        std::cerr << "Failed to load initial resources." << std::endl;
        return false;
    }

    // Initialize InputManager after WorldMap is loaded
    inputManager = std::make_unique<InputManager>(eventManager, camera, windowManager->GetWindow(), worldMap);
    inputManager->SetZoomSpeed(Constants::CAMERA_ZOOM_SPEED);  // Set camera zoom speed
    inputManager->SetPanSpeed(Constants::CAMERA_PAN_SPEED);  // Set camera pan speed

    // Subscribe to Window Close event
    eventManager->Subscribe(EventType::Closed, [this](const sf::Event& event) {
        isRunning = false;
        windowManager->GetWindow().close();
        });

    // Subscribe to Window Resize event and adjust camera view accordingly
    eventManager->Subscribe(EventType::Resized, [this](const sf::Event& event) {
        if (event.type == sf::Event::Resized) {
            sf::Vector2u newSize(event.size.width, event.size.height);
            camera->OnResize(newSize);  // Notify camera of the new window size
            sf::FloatRect visibleArea(0, 0, static_cast<float>(newSize.x), static_cast<float>(newSize.y));
            windowManager->GetWindow().setView(sf::View(visibleArea));
        }
        });

    // Initialize UIManager for managing GUI elements
    uiManager = std::make_shared<UIManager>(worldMap);
    uiManager->SetWindow(windowManager->GetWindow());
    if (!uiManager->Init()) {
        std::cerr << "Failed to initialize UIManager." << std::endl;
        return false;
    }

    // Initialize Renderer with WorldMap and configure camera
    if (worldMap) {
        camera->SetZoom(Constants::CAMERA_MAX_ZOOM);  // Set initial camera zoom level
        camera->SetWorldBounds(worldMap->GetWorldWidth(), worldMap->GetWorldHeight());  // Set camera boundaries to match world size
        camera->SetPosition(sf::Vector2f(worldMap->GetWorldWidth() / 2.0f, worldMap->GetWorldHeight() / 2.0f));  // Center camera on the world

        // Set WorldMap in Renderer
        renderer->SetWorldMap(worldMap);
    }
    else {
        std::cerr << "WorldMap is not loaded." << std::endl;
        return false;
    }

    isRunning = true;  // Set game state to running
    return true;
}

// Initialize all required managers (WindowManager, etc.)
bool Game::InitManagers() {
    // Initialize WindowManager and configure it with settings
    auto windowMgr = std::make_shared<WindowManager>();
    windowMgr->SetVideoMode(sf::VideoMode(videoMode));  // Set window video mode
    windowMgr->SetTitle(windowTitle);  // Set window title

    // Enable fullscreen mode if desired (currently set to true)
    windowMgr->SetFullscreen(true);  // Set to false if you want windowed mode

    // Configure advanced graphics settings for the window
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;  // Set antialiasing level for smoother graphics
    settings.depthBits = 24;  // Set depth buffer bits
    settings.stencilBits = 8;  // Set stencil buffer bits
    settings.majorVersion = 3;  // Set OpenGL version (major)
    settings.minorVersion = 0;  // Set OpenGL version (minor)
    windowMgr->SetContextSettings(settings);

    if (!windowMgr->Init()) return false;  // Initialize window and check for success
    initManager.Register(windowMgr);  // Register WindowManager with InitializationManager
    windowManager = windowMgr;

    // Register other initializable modules and initialize all
    return initManager.InitAll();
}

// Load resources needed for the game asynchronously
bool Game::LoadResources() {
    std::condition_variable cv;
    bool loaded = false;
    std::mutex cvMutex;

    // Create a task to load the WorldMap and enqueue it to ThreadPool
    Task loadWorldMapTask([this, &cv, &loaded]() {
        auto tempWorldMap = std::make_shared<WorldMap>("assets/land_shapes.json", "assets/features/cities.geojson", "assets/features/towns.geojson", "assets/features/suburbs.geojson");
        if (tempWorldMap->Init()) {
            {
                std::lock_guard<std::mutex> lock(worldMapMutex);  // Lock mutex before updating shared resource
                worldMap = tempWorldMap;  // Assign the loaded WorldMap
                loaded = true;  // Set loaded flag to true
            }
            cv.notify_one();  // Notify waiting thread that WorldMap is loaded
        }
        else {
            std::cerr << "Failed to initialize WorldMap." << std::endl;
        }
        });
    threadPool->enqueueTask(loadWorldMapTask);  // Enqueue the task for loading WorldMap

    // Wait for WorldMap to load using a condition variable
    std::unique_lock<std::mutex> lock(cvMutex);
    cv.wait(lock, [&loaded]() { return loaded; });

    // Configure Camera based on WorldMap dimensions
    if (worldMap) {
        camera->SetZoom(Constants::CAMERA_MAX_ZOOM);  // Set maximum zoom level
        camera->SetWorldBounds(worldMap->GetWorldWidth(), worldMap->GetWorldHeight());  // Set camera boundaries to match world size
        camera->SetPosition(sf::Vector2f(worldMap->GetWorldWidth() / 2.0f, worldMap->GetWorldHeight() / 2.0f));  // Center camera on the world

        // Set WorldMap in Renderer
        renderer->SetWorldMap(worldMap);
    }
    else {
        std::cerr << "WorldMap is not loaded." << std::endl;
        return false;
    }

    return true;
}

// Main game loop
void Game::Run() {
    while (isRunning && windowManager->IsOpen()) {  // Loop until the game is no longer running or window is closed
        ProcessEvents();  // Handle all pending events

        // Calculate delta time (time since last frame)
        float dt = deltaClock.restart().asSeconds();

        // Update game logic based on delta time
        Update(dt);

        // Render the current frame
        Render();
    }
}

// Handle input and window events
void Game::ProcessEvents() {
    sf::Event event;
    while (windowManager->PollEvent(event)) {  // Poll events from WindowManager
        eventManager->Dispatch(event);  // Dispatch events to relevant handlers
        uiManager->ProcessEvent(event);  // Pass events to UIManager for GUI handling
    }
}

// Update game logic, including camera, input, UI, and game world elements
void Game::Update(float dt) {
    if (camera) {
        camera->Update(dt);  // Update camera position and zoom based on input
    }

    if (inputManager) {
        inputManager->HandleInput(dt);  // Process player input (keyboard, mouse, etc.)
    }

    if (uiManager) {
        uiManager->Update(dt);  // Update UI elements
    }

    // Update game objects such as trains on the map
    if (worldMap) {
        auto& lines = worldMap->GetLines();  // Get all lines in the WorldMap
        for (auto& line : lines) {
            auto& trains = line.GetTrains();  // Get all trains for each line
            for (auto& train : trains) {
                train.Update(dt);  // Update each train's position and state
            }
        }
    }
}

// Render game and UI elements
void Game::Render() {
    // Clear the window with a background color
    windowManager->Clear(sf::Color(174, 223, 246));  // Clear with sky-blue color

    // Apply camera view to the window to render the game world
    if (camera) {
        camera->ApplyView(windowManager->GetWindow());
    }

    // Render all game elements via the Renderer
    if (renderer) {
        renderer->Render(windowManager->GetWindow(), *camera);
    }

    // Reset to default view to render UI elements
    windowManager->GetWindow().setView(windowManager->GetWindow().getDefaultView());

    // Render UI elements like HUD, menus, etc.
    if (uiManager) {
        uiManager->Render();
    }

    // Display the rendered frame on the screen
    windowManager->Display();
}

// Clean up resources and shutdown the game properly
void Game::Shutdown() {
    if (isRunning) {
        isRunning = false;  // Set game state to not running
    }

    // Shutdown Renderer to release graphics resources
    if (renderer) {
        renderer->Shutdown();
    }

    // Shutdown ThreadPool to stop all threads
    if (threadPool) {
        threadPool->shutdown();
    }

    // Shutdown UIManager to clean up GUI-related resources
    if (uiManager) {
        uiManager->Shutdown();
    }

    // Clean up other modules by resetting smart pointers
    renderer.reset();
    threadPool.reset();
    inputManager.reset();
    camera.reset();
    worldMap.reset();
    windowManager.reset();
    uiManager.reset();
}
