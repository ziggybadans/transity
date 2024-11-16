#include "Game.h"
#include <iostream>
#include <future>
#include <thread>
#include <condition_variable>
#include "Constants.h"

Game::Game()
    : m_videoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT)
    , m_windowTitle(Constants::WINDOW_TITLE)
    , m_isRunning(false)
    , m_timeScale(1.0f)
{}

Game::~Game() {
    Shutdown();
}

bool Game::Init() {
    if (!InitManagers()) {
        std::cerr << "Failed to initialize managers." << std::endl;
        return false;
    }

    m_eventManager = std::make_shared<EventManager>();

    sf::Vector2u windowSize = m_windowManager->GetWindow().getSize();
    m_camera = std::make_shared<Camera>(windowSize);
    m_camera->SetMinZoomLevel(Constants::CAMERA_MIN_ZOOM);
    m_camera->SetMaxZoomLevel(Constants::CAMERA_MAX_ZOOM);

    m_threadPool = std::make_unique<ThreadPool>(std::thread::hardware_concurrency());

    m_renderer = std::make_unique<Renderer>();
    if (!m_renderer->Init(m_windowManager->GetWindow())) {
        std::cerr << "Failed to initialize Renderer." << std::endl;
        return false;
    }
    m_renderer->SetWorldMap(m_worldMap);

    if (!LoadResources()) {
        std::cerr << "Failed to load initial resources." << std::endl;
        return false;
    }

    m_inputManager = std::make_shared<InputManager>(m_eventManager, m_windowManager->GetWindow());
    m_inputManager->SetZoomSpeed(Constants::CAMERA_ZOOM_SPEED);
    m_inputManager->SetPanSpeed(Constants::CAMERA_PAN_SPEED);

    m_uiManager = std::make_shared<UIManager>(m_worldMap);
    m_uiManager->SetWindow(m_windowManager->GetWindow());
    if (!m_uiManager->Init()) {
        std::cerr << "Failed to initialize UIManager." << std::endl;
        return false;
    }

    m_isRunning = true;
    return true;
}

bool Game::InitManagers() {
    auto windowMgr = std::make_shared<WindowManager>();
    windowMgr->SetVideoMode(sf::VideoMode(m_videoMode));
    windowMgr->SetTitle(m_windowTitle);

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.majorVersion = 3;
    settings.minorVersion = 0;
    windowMgr->SetContextSettings(settings);

    if (!windowMgr->Init()) {
        return false;
    }
    
    m_initManager.Register(std::static_pointer_cast<IInitializable>(windowMgr));
    m_windowManager = windowMgr;

    return m_initManager.InitAll();
}

bool Game::LoadResources() {
    std::condition_variable cv;
    bool loaded = false;
    std::mutex cvMutex;

    Task loadWorldMapTask([this, &cv, &loaded]() {
        auto tempWorldMap = std::make_shared<WorldMap>(
            "assets/land_shapes.json",
            "assets/features/cities.geojson",
            "assets/features/towns.geojson",
            "assets/features/suburbs.geojson"
        );
        
        if (tempWorldMap->Init()) {
            {
                std::lock_guard<std::mutex> lock(m_worldMapMutex);
                m_worldMap = tempWorldMap;
                loaded = true;
            }
            cv.notify_one();
        }
        else {
            std::cerr << "Failed to initialize WorldMap." << std::endl;
        }
    });
    
    m_threadPool->enqueueTask(loadWorldMapTask);

    std::unique_lock<std::mutex> lock(cvMutex);
    cv.wait(lock, [&loaded]() { return loaded; });

    if (m_worldMap) {
        m_camera->SetZoom(Constants::CAMERA_MAX_ZOOM);
        m_camera->SetWorldBounds(m_worldMap->GetWorldWidth(), m_worldMap->GetWorldHeight());
        m_camera->SetPosition(sf::Vector2f(
            m_worldMap->GetWorldWidth() / 2.0f,
            m_worldMap->GetWorldHeight() / 2.0f
        ));
    }
    else {
        std::cerr << "WorldMap is not loaded." << std::endl;
        return false;
    }

    return true;
}

void Game::Run() {
    while (m_isRunning && m_windowManager->GetWindow().isOpen()) {
        ProcessEvents();

        float dt = m_deltaClock.restart().asSeconds();
        UpdateNonSimulation(dt);

        float scaledDt = dt * m_timeScale.load();
        Render();
    }
}

void Game::ProcessEvents() {
    sf::Event event;
    while (m_windowManager->GetWindow().pollEvent(event)) {
        m_eventManager->Dispatch(event);
        m_uiManager->ProcessEvent(event);
    }
}

void Game::UpdateNonSimulation(float dt) {
    if (m_camera) {
        m_camera->Update(dt);
    }

    if (m_inputManager) {
        m_inputManager->HandleInput(dt);
    }

    if (m_uiManager) {
        m_uiManager->Update(dt);
    }
}

void Game::Render() {
    m_windowManager->GetWindow().clear(sf::Color(174, 223, 246));

    if (m_camera) {
        m_camera->ApplyView(m_windowManager->GetWindow());
    }

    if (m_renderer) {
        m_renderer->Render(m_windowManager->GetWindow(), *m_camera);
    }

    m_windowManager->GetWindow().setView(m_windowManager->GetWindow().getDefaultView());

    if (m_uiManager) {
        m_uiManager->Render();
    }

    m_windowManager->GetWindow().display();
}

void Game::Shutdown() {
    if (m_isRunning) {
        m_isRunning = false;
    }

    if (m_renderer) {
        m_renderer->Shutdown();
    }

    if (m_threadPool) {
        m_threadPool->shutdown();
    }

    if (m_uiManager) {
        m_uiManager->Shutdown();
    }

    m_renderer.reset();
    m_threadPool.reset();
    m_inputManager.reset();
    m_camera.reset();
    m_worldMap.reset();
    m_windowManager.reset();
    m_uiManager.reset();
}