#include "Game.h"
#include <iostream>
#include <future>
#include <thread>
#include <condition_variable>
#include "Constants.h"
#include "Debug.h"
#include "utility/Profiler.h"

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
            DEBUG_ERROR("Failed to initialize managers.");
            return false;
        }

        m_eventManager = std::make_shared<EventManager>();
        if (!m_windowManager) {
            DEBUG_ERROR("Window manager not initialized.");
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
            DEBUG_ERROR("Failed to load initial resources.");
            return false;
        }

        m_renderer = std::make_unique<Renderer>();
        if (!m_renderer->Init() || !m_renderer->InitWithWindow(m_windowManager->GetWindow())) {
            DEBUG_ERROR("Failed to initialize Renderer.");
            return false;
        }

        m_inputManager = std::make_shared<InputManager>(m_eventManager, m_windowManager->GetWindow());
        m_inputManager->SetZoomSpeed(Constants::CAMERA_ZOOM_SPEED);
        m_inputManager->SetPanSpeed(Constants::CAMERA_PAN_SPEED);

        auto actionRegistrar = std::make_unique<ActionRegistrar>(m_inputManager, m_camera);
        actionRegistrar->RegisterActions();
        m_actionRegistrar = std::move(actionRegistrar);

        m_uiManager = std::make_shared<UIManager>();
        m_uiManager->SetWindow(m_windowManager->GetWindow());
        m_gameSettings = std::make_shared<GameSettings>();
        if (!m_gameSettings->LoadSettings("config/settings.json")) {
            DEBUG_WARNING("Failed to load game settings, using defaults.");
        }
        m_uiManager->SetGameSettings(m_gameSettings);
        m_uiManager->SetWindowManager(m_windowManager);
        if (!m_uiManager->Init()) {
            DEBUG_ERROR("Failed to initialize UIManager.");
            return false;
        }

        m_pluginManager = std::make_unique<PluginManager>();

        m_saveManager = std::make_unique<SaveManager>();
        m_saveManager->SetGameSettings(m_gameSettings);

        m_isRunning = true;

        // Enable debug logging
        Debug::EnableFileLogging("game.log");
        Debug::SetLevel(DebugLevel::Debug);  // Set default debug level
        
        DEBUG_INFO("Initializing game...");

        m_uiManager->SetInputManager(m_inputManager);

        return true;
    } catch (const std::exception& e) {
        DEBUG_ERROR("Error during initialization: ", e.what());
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
        PROFILE_SCOPE("Game Loop");
        {
            PROFILE_SCOPE("Event Processing");
            ProcessEvents();
        }

        float dt = m_deltaClock.restart().asSeconds();
        {
            PROFILE_SCOPE("Non-Simulation Update");
            UpdateNonSimulation(dt);
        }

        float scaledDt = dt * m_timeScale.load();
        {
            PROFILE_SCOPE("Render");
            Render();
        }
    }
}

void Game::ProcessEvents() {
    sf::Event event;
    while (m_windowManager->PollEvent(event)) {
        std::invoke([this, &event] {
            m_eventManager->Dispatch(event);
            m_uiManager->ProcessEvent(event);
        });
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

    if (m_pluginManager) {
        m_pluginManager->UpdatePlugins(dt);
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
    m_resourceManager.reset();
    m_windowManager.reset();
    m_uiManager.reset();
    m_pluginManager.reset();
}