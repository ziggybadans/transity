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

Game::Game()
    : videoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT),
    windowTitle(Constants::WINDOW_TITLE),
    isRunning(false) {}

Game::~Game() {
    Shutdown();
}

bool Game::Init() {
    if (!InitManagers()) {
        std::cerr << "Failed to initialize managers." << std::endl;
        return false;
    }

    // Initialize EventManager
    eventManager = std::make_shared<EventManager>();

    // Initialize Camera
    sf::Vector2u windowSize = windowManager->GetWindow().getSize();
    camera = std::make_shared<Camera>(windowSize);
    camera->setMinZoomLevel(Constants::CAMERA_MIN_ZOOM);
    camera->setMaxZoomLevel(Constants::CAMERA_MAX_ZOOM);

    // Initialize InputManager
    inputManager = std::make_unique<InputManager>(eventManager, camera, windowManager->GetWindow());
    inputManager->SetZoomSpeed(Constants::CAMERA_ZOOM_SPEED);
    inputManager->SetPanSpeed(Constants::CAMERA_PAN_SPEED);
    inputManager->SetMinZoom(Constants::CAMERA_MIN_ZOOM);
    inputManager->SetMaxZoom(Constants::CAMERA_MAX_ZOOM);

    // Subscribe to Events
    eventManager->Subscribe(EventType::Closed, [this](const sf::Event& event) {
        isRunning = false;
        windowManager->GetWindow().close();
        });

    eventManager->Subscribe(EventType::Resized, [this](const sf::Event& event) {
        if (event.type == sf::Event::Resized) {
            sf::Vector2u newSize(event.size.width, event.size.height);
            camera->OnResize(newSize);
            sf::FloatRect visibleArea(0, 0, static_cast<float>(newSize.x), static_cast<float>(newSize.y));
            windowManager->GetWindow().setView(sf::View(visibleArea));
        }
        });

    // Initialize ThreadPool
    threadPool = std::make_unique<ThreadPool>(std::thread::hardware_concurrency());

    // Initialize Renderer
    renderer = std::make_unique<Renderer>();
    if (!renderer->Init(windowManager->GetWindow(), *threadPool)) {
        std::cerr << "Failed to initialize Renderer." << std::endl;
        return false;
    }

    // Load initial resources asynchronously
    if (!LoadResources()) {
        std::cerr << "Failed to load initial resources." << std::endl;
        return false;
    }

    isRunning = true;
    return true;
}

bool Game::InitManagers() {
    // Initialize WindowManager
    auto windowMgr = std::make_shared<WindowManager>();
    windowMgr->SetVideoMode(sf::VideoMode(videoMode));
    windowMgr->SetTitle(windowTitle);

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8; // Ensure this is set to a high enough value
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.majorVersion = 3;
    settings.minorVersion = 0;
    windowMgr->SetContextSettings(settings);

    if (!windowMgr->Init()) return false;
    initManager.Register(windowMgr);
    windowManager = windowMgr;

    // Register other initializable modules
    return initManager.InitAll();
}

bool Game::LoadResources() {
    std::condition_variable cv;
    bool loaded = false;
    std::mutex cvMutex;

    // Enqueue WorldMap loading task
    Task loadWorldMapTask([this, &cv, &loaded]() {
        std::string highResPath = "assets/world_high_detail.png";
        std::string lowResPath = "assets/world_low_detail.png";
        float zoomSwitch = 1.0f; // Example threshold, adjust as needed

        auto tempWorldMap = std::make_shared<WorldMap>(highResPath, lowResPath, zoomSwitch);
        if (tempWorldMap->Init()) {
            {
                std::lock_guard<std::mutex> lock(worldMapMutex);
                worldMap = tempWorldMap;
                loaded = true;
            }
            cv.notify_one();
        }
        else {
            std::cerr << "Failed to initialize WorldMap." << std::endl;
        }
        });
    threadPool->enqueueTask(loadWorldMapTask);

    // Wait for WorldMap to load
    std::unique_lock<std::mutex> lock(cvMutex);
    cv.wait(lock, [&loaded]() { return loaded; });

    // Configure Camera based on WorldMap
    if (worldMap) {
        camera->SetZoom(Constants::CAMERA_MAX_ZOOM);
        camera->SetWorldBounds(worldMap->GetWorldWidth(), worldMap->GetWorldHeight());
        camera->SetPosition(sf::Vector2f(worldMap->GetWorldWidth() / 2.0f, worldMap->GetWorldHeight() / 2.0f));

        // Set WorldMap in Renderer
        renderer->SetWorldMap(worldMap);
    }
    else {
        std::cerr << "WorldMap is not loaded." << std::endl;
        return false;
    }

    // Enqueue CityManager loading task
    std::condition_variable cvCities;
    bool citiesLoaded = false;
    std::mutex cvCitiesMutex;

    Task loadCitiesTask([this, &cvCities, &citiesLoaded]() {
        std::string geonamesPath = "assets/cities1000.txt"; // Example path
        auto tempCityManager = std::make_unique<CityManager>(geonamesPath, *worldMap);
        if (tempCityManager->Init()) {
            {
                std::lock_guard<std::mutex> lock(worldMapMutex);
                cityManager = std::move(tempCityManager);
                citiesLoaded = true;
            }
            cvCities.notify_one();
        }
        else {
            std::cerr << "Failed to initialize CityManager." << std::endl;
        }
        });
    threadPool->enqueueTask(loadCitiesTask);

    // Wait for CityManager to load
    std::unique_lock<std::mutex> lockCities(cvCitiesMutex);
    cvCities.wait(lockCities, [&citiesLoaded]() { return citiesLoaded; });

    // Initialize the circle shape for cities
    if (cityManager) {
        cityCircleShape = std::make_shared<sf::CircleShape>(5.0f); // Radius 5, adjust as needed
        cityCircleShape->setFillColor(sf::Color::White);
        cityCircleShape->setOutlineThickness(2.0f);
        cityCircleShape->setOutlineColor(sf::Color::Black);
        cityCircleShape->setOrigin(5.0f, 5.0f); // Center the circle
    }
    else {
        std::cerr << "CityManager is not loaded." << std::endl;
        return false;
    }

    // Set CityManager and city shape in Renderer
    renderer->SetCityManager(cityManager.get());
    renderer->SetCityCircleShape(cityCircleShape);

    // Load and set the font in Renderer
    if (!renderer->Init(windowManager->GetWindow(), *threadPool)) {
        std::cerr << "Failed to initialize Renderer." << std::endl;
        return false;
    }

    isRunning = true;
    return true;
}

void Game::Run() {
    while (isRunning && windowManager->IsOpen()) {
        ProcessEvents();

        // Calculate delta time
        float dt = deltaClock.restart().asSeconds();

        // Update game logic
        Update(dt);

        // Render the frame
        Render();
    }
}

void Game::ProcessEvents() {
    sf::Event event;
    while (windowManager->PollEvent(event)) {
        eventManager->Dispatch(event);
    }
}

void Game::Update(float dt) {
    if (camera) {
        camera->Update(dt);
    }

    if (inputManager) {
        inputManager->HandleInput(dt);
    }

    // Update other game states here
}

void Game::Render() {
    // Clear the window
    windowManager->Clear(sf::Color::Black);

    // Apply camera view
    if (camera) {
        camera->ApplyView(windowManager->GetWindow());
    }

    // Render game elements via Renderer
    if (renderer) {
        renderer->Render(windowManager->GetWindow(), *camera);
    }

    // Render UI elements if applicable
    if (uiManager) {
        uiManager->Render();
    }

    // Display the rendered frame
    windowManager->Display();
}

void Game::Shutdown() {
    if (isRunning) {
        isRunning = false;
    }

    // Shutdown Renderer
    if (renderer) {
        renderer->Shutdown();
    }

    // Shutdown ThreadPool
    if (threadPool) {
        threadPool->shutdown();
    }

    // Shutdown UIManager
    if (uiManager) {
        uiManager->Shutdown();
    }

    // Clean up other modules
    renderer.reset();
    threadPool.reset();
    inputManager.reset();
    camera.reset();
    worldMap.reset();
    windowManager.reset();
    uiManager.reset();
}
