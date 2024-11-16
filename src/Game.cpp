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
    try {
        if (!InitManagers()) {
            std::cerr << "Failed to initialize managers." << std::endl;
            return false;
        }

        m_eventManager = std::make_shared<EventManager>();
        if (!m_windowManager) {
            std::cerr << "Window manager not initialized." << std::endl;
            return false;
        }

        sf::Vector2u windowSize = m_windowManager->GetWindow().getSize();
        m_camera = std::make_shared<Camera>(windowSize);
        m_camera->SetMinZoomLevel(Constants::CAMERA_MIN_ZOOM);
        m_camera->SetMaxZoomLevel(Constants::CAMERA_MAX_ZOOM);

        unsigned int threadCount = std::max(1u, std::thread::hardware_concurrency());
        m_threadPool = std::make_unique<ThreadPool>(threadCount);

        m_resourceManager = std::make_shared<ResourceManager>(m_threadPool);
        if (!m_resourceManager->LoadResources()) {
            std::cerr << "Failed to load initial resources." << std::endl;
            return false;
        }

        m_worldMap = m_resourceManager->GetWorldMap();
        
        if (m_worldMap) {
            m_camera->SetZoom(Constants::CAMERA_MAX_ZOOM);
            m_camera->SetWorldBounds(m_worldMap->GetWorldWidth(), m_worldMap->GetWorldHeight());
            m_camera->SetPosition(sf::Vector2f(
                m_worldMap->GetWorldWidth() / 2.0f,
                m_worldMap->GetWorldHeight() / 2.0f
            ));
            std::cout << "Camera initialized to center of the world." << std::endl;
        }

        m_renderer = std::make_unique<Renderer>();
        m_renderer->SetWorldMap(m_worldMap);
        if (!m_renderer->Init(m_windowManager->GetWindow())) {
            std::cerr << "Failed to initialize Renderer." << std::endl;
            return false;
        }

        m_inputManager = std::make_shared<InputManager>(m_eventManager, m_windowManager->GetWindow());
        m_inputManager->SetZoomSpeed(Constants::CAMERA_ZOOM_SPEED);
        m_inputManager->SetPanSpeed(Constants::CAMERA_PAN_SPEED);

        auto actionRegistrar = std::make_unique<ActionRegistrar>(m_inputManager, m_camera);
        actionRegistrar->RegisterActions();
        m_actionRegistrar = std::move(actionRegistrar);

        m_uiManager = std::make_shared<UIManager>(m_worldMap);
        m_uiManager->SetWindow(m_windowManager->GetWindow());
        if (!m_uiManager->Init()) {
            std::cerr << "Failed to initialize UIManager." << std::endl;
            return false;
        }

        m_isRunning = true;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error during initialization: " << e.what() << std::endl;
        return false;
    }
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

    if (m_uiManager) {
        m_uiManager->Update(dt);
    }

    if (m_inputManager) {
        m_inputManager->HandleInput(dt);
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
    m_isRunning = false;

    if (m_uiManager) {
        m_uiManager->Shutdown();
    }
    if (m_renderer) {
        m_renderer->Shutdown();
    }
    if (m_threadPool) {
        m_threadPool->shutdown();
    }

    m_renderer.reset();
    m_threadPool.reset();
    m_inputManager.reset();
    m_camera.reset();
    m_worldMap.reset();
    m_windowManager.reset();
    m_uiManager.reset();
    m_resourceManager.reset();
}