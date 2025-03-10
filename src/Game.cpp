#include "Game.h"

#include <iostream>
#include <future>
#include <thread>
#include <condition_variable>

#include "Constants.h"
#include "Debug.h"
#include "utility/Profiler.h"
#include "core/ThreadManager.h"
#include "settings/SettingsDefinitions.h"
#include "settings/SettingsRegistry.h"

Game::Game()
    : m_videoMode(Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT)
    , m_windowTitle(Constants::WINDOW_TITLE)
    , m_timeScale(1.0f)
{}

Game::~Game() {
    Shutdown();
}

bool Game::Init() {
    try {
        Debug::EnableFileLogging("game.log"); // Enable debug logging
        Debug::SetLevel(DebugLevel::Debug);  // Set default debug level
        DEBUG_INFO("Initializing game...");

        // Stage 1: Utilities and core systems
        m_stateManager = std::make_shared<StateManager>();
        m_stateManager->InitializeCoreStates();
        m_stateManager->SetState("Loading", true);

        m_threadManager = std::make_unique<ThreadManager>(std::thread::hardware_concurrency());
        m_eventManager = std::make_shared<EventManager>();

        m_resourceManager = std::make_shared<ResourceManager>(m_threadManager);
        if (!m_resourceManager->LoadResources()) {
            DEBUG_ERROR("Failed to load initial resources.");
            return false;
        }

        RegisterSettings(); // Settings registry

        m_gameSettings = std::make_shared<GameSettings>();
        if (!m_gameSettings->LoadSettings("config/settings.json")) {
            DEBUG_WARNING("Failed to load game settings, using defaults.");
        }

        m_pluginManager = std::make_unique<PluginManager>();

        // Stage 2: Graphics
        auto windowMgr = std::make_shared<WindowManager>();
        windowMgr->SetVideoMode(sf::VideoMode(m_videoMode));
        windowMgr->SetTitle(m_windowTitle);

        sf::ContextSettings settings;
        settings.antialiasingLevel = 8;
        settings.depthBits = 24;
        settings.stencilBits = 8;
        settings.majorVersion = 3; settings.minorVersion = 0; // OpenGL version to use
        windowMgr->SetContextSettings(settings);

        if (!windowMgr->Init()) { return false; }
        m_windowManager = windowMgr;

        sf::Vector2u windowSize = m_windowManager->GetWindow().getSize();
        m_camera = std::make_shared<Camera>(windowSize);
        m_camera->SetMinZoomLevel(Constants::CAMERA_MIN_ZOOM);
        m_camera->SetMaxZoomLevel(Constants::CAMERA_MAX_ZOOM);

        m_renderer = std::make_unique<Renderer>();
        if (!m_renderer->Init() || !m_renderer->InitWithWindow(m_windowManager->GetWindow())) { // Tries to start the renderer even if the window hasn't been fully initialized yet
            DEBUG_ERROR("Failed to initialize Renderer.");
            return false;
        }

        // Stage 3: World
        InitializeWorld();
        m_simulation = std::make_unique<Simulation>(m_map);
        m_saveManager = std::make_shared<SaveManager>();
        m_saveManager->SetGameSettings(m_gameSettings);
        m_saveManager->SetWorld(m_map);

        // Stage 4: Inputs
        m_inputManager = std::make_shared<InputManager>(m_eventManager, m_stateManager, m_windowManager->GetWindow(), m_camera, m_map);

        m_uiManager = std::make_shared<UIManager>();
        m_uiManager->SetWindow(m_windowManager->GetWindow());
        m_uiManager->SetGameSettings(m_gameSettings);
        m_uiManager->SetWindowManager(m_windowManager);
        m_uiManager->SetInputManager(m_inputManager);
        m_uiManager->SetStateManager(m_stateManager);
        m_uiManager->SetMap(m_map);
        m_uiManager->SetSaveManager(m_saveManager);

        // Set time scale callbacks
        m_uiManager->SetTimeScaleCallback(
            [this](float newScale) { this->SetTimeScale(newScale); }, // Setter
            [this]() -> float { return this->GetTimeScale(); }        // Getter
        );

        if (!m_uiManager->Init()) {
            DEBUG_ERROR("Failed to initialize UIManager.");
            return false;
        }

        // Stage 5: Finish initialization
        m_stateManager->SetState("Loading", false);
        m_stateManager->SetState("Running", true);

        return true;
    } catch (const std::exception& e) {
        DEBUG_ERROR("Error during initialization: ", e.what());
        return false;
    }
}

void Game::InitializeWorld() {
    m_map = std::make_shared<Map>(Constants::MAP_SIZE, *m_stateManager);

    m_stateManager->RegisterState("CurrentTool");
    m_stateManager->SetState("CurrentTool", std::string("Place"));
    m_stateManager->RegisterState("TrainPlaceVerified");
    m_stateManager->SetState("TrainPlaceVerified", false);
}

void Game::Run()
{
    while (m_stateManager->GetState<bool>("Running")) {
        std::shared_ptr<WindowManager> windowMgrCopy;
        {
            std::lock_guard<std::mutex> lock(m_windowManagerMutex);
            windowMgrCopy = m_windowManager;
        }

        if (!windowMgrCopy || !windowMgrCopy->GetWindow().isOpen()) {
            break;
        }

        PROFILE_SCOPE("Game Loop");

        {
            PROFILE_SCOPE("Event Processing");
            ProcessEvents();
        }

        float dt = m_deltaClock.restart().asSeconds();
        float currentTimeScale = m_timeScale.load();

        {
            PROFILE_SCOPE("Non-Simulation Update");
            UpdateNonSimulation(dt);
        }

        if (currentTimeScale != 0.0f) { // Only update simulation if not paused
            float scaledDt = dt * currentTimeScale;
            {
                PROFILE_SCOPE("Simulation Update");
                m_simulation->Update(scaledDt);
            }
        }

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
            if (event.type == sf::Event::Closed) { Shutdown(); }
        });
    }
}

void Game::UpdateNonSimulation(float dt) {
    if (m_camera) { m_camera->Update(dt); }
    if (m_uiManager) { m_uiManager->Update(dt); }
    if (m_inputManager) { m_inputManager->HandleInput(dt); }
    if (m_pluginManager) { m_pluginManager->UpdatePlugins(dt); }
}

void Game::Render() {
    m_windowManager->GetWindow().clear(sf::Color(174, 223, 246));

    if (m_camera) {
        m_camera->ApplyView(m_windowManager->GetWindow());
    }

    if (m_renderer) {
        m_renderer->Render(m_windowManager->GetWindow(), *m_camera, *m_map, *m_stateManager);
    }

    m_windowManager->GetWindow().setView(m_windowManager->GetWindow().getDefaultView());

    if (m_uiManager) {
        m_uiManager->Render();
    }

    m_windowManager->GetWindow().display();
}

std::atomic<bool> m_isShuttingDown{ false };

void Game::Shutdown() {
    bool expected = false;
    if (!m_isShuttingDown.compare_exchange_strong(expected, true)) {
        // Shutdown already in progress
        return;
    }

    // First, set "Running" to false to ensure the loop exits
    m_stateManager->SetState("Running", false);

    {
        std::lock_guard<std::mutex> lock(m_windowManagerMutex);
        if (m_windowManager) {
            m_windowManager->GetWindow().close();
        }
    }

    // Proceed with shutting down other managers
    if (m_uiManager) { m_uiManager->Shutdown(); }
    if (m_renderer) { m_renderer->Shutdown(); }
    if (m_threadManager) { m_threadManager->Shutdown(); }

    // Reset managers after ensuring the loop has exited
    m_renderer.reset();
    m_threadManager.reset();
    m_inputManager.reset();
    m_camera.reset();
    m_resourceManager.reset();
    m_uiManager.reset();
    m_pluginManager.reset();
    
    {
        std::lock_guard<std::mutex> lock(m_windowManagerMutex);
        m_windowManager.reset();
    }
}

void Game::RegisterSettings() {
    auto& registry = SettingsRegistry::Instance();

    // Video Settings
    registry.RegisterSetting({
        Settings::Names::RESOLUTION,
        Settings::Categories::VIDEO,
        SettingType::Vector2u,
        Settings::Defaults::RESOLUTION,
        [this](const std::any& value) {
            auto resolution = std::any_cast<sf::Vector2u>(value);
            if (m_windowManager) {
                m_windowManager->SetVideoMode(sf::VideoMode(resolution.x, resolution.y));
                m_windowManager->ApplyVideoMode();
            }
        }
    });

    registry.RegisterSetting({
        Settings::Names::FULLSCREEN,
        Settings::Categories::VIDEO,
        SettingType::Boolean,
        Settings::Defaults::FULLSCREEN,
        [this](const std::any& value) {
            if (m_windowManager) {
                m_windowManager->SetFullscreen(std::any_cast<bool>(value));
                m_windowManager->ApplyVideoMode();
            }
        }
    });

    registry.RegisterSetting({
        Settings::Names::VSYNC,
        Settings::Categories::VIDEO,
        SettingType::Boolean,
        Settings::Defaults::VSYNC,
        [this](const std::any& value) {
            if (m_windowManager) {
                m_windowManager->SetVSync(std::any_cast<bool>(value));
            }
        }
    });

    registry.RegisterSetting({
        Settings::Names::FRAME_RATE_LIMIT,
        Settings::Categories::VIDEO,
        SettingType::Integer,
        Settings::Defaults::FRAME_RATE_LIMIT,
        [this](const std::any& value) {
            auto limit = std::any_cast<unsigned int>(value);
            if (m_windowManager) {
                m_windowManager->SetFramerateLimit(limit);
            }
        },
        [](const std::any& value) {
            auto limit = std::any_cast<unsigned int>(value);
            return limit >= 30 && limit <= 240;
        }
    });

    // Audio Settings
    registry.RegisterSetting({
        Settings::Names::MASTER_VOLUME,
        Settings::Categories::AUDIO,
        SettingType::Float,
        Settings::Defaults::MASTER_VOLUME,
        nullptr,
        [](const std::any& value) {
            auto volume = std::any_cast<float>(value);
            return volume >= 0.0f && volume <= 1.0f;
        }
    });

    registry.RegisterSetting({
        Settings::Names::MUSIC_VOLUME,
        Settings::Categories::AUDIO,
        SettingType::Float,
        Settings::Defaults::MUSIC_VOLUME,
        nullptr,
        [](const std::any& value) {
            auto volume = std::any_cast<float>(value);
            return volume >= 0.0f && volume <= 1.0f;
        }
    });

    registry.RegisterSetting({
        Settings::Names::SFX_VOLUME,
        Settings::Categories::AUDIO,
        SettingType::Float,
        Settings::Defaults::SFX_VOLUME,
        nullptr,
        [](const std::any& value) {
            auto volume = std::any_cast<float>(value);
            return volume >= 0.0f && volume <= 1.0f;
        }
    });

    // Gameplay Settings
    registry.RegisterSetting({
        Settings::Names::CAMERA_ZOOM_SPEED,
        Settings::Categories::GAMEPLAY,
        SettingType::Float,
        Settings::Defaults::CAMERA_ZOOM_SPEED,
        [this](const std::any& value) {
            auto speed = std::any_cast<float>(value);
        },
        [](const std::any& value) {
            auto speed = std::any_cast<float>(value);
            return speed >= 1.0f && speed <= 2.0f;
        }
    });

    registry.RegisterSetting({
        Settings::Names::CAMERA_PAN_SPEED,
        Settings::Categories::GAMEPLAY,
        SettingType::Float,
        Settings::Defaults::CAMERA_PAN_SPEED,
        [this](const std::any& value) {
            auto speed = std::any_cast<float>(value);
        },
        [](const std::any& value) {
            auto speed = std::any_cast<float>(value);
            return speed >= 100.0f && speed <= 1000.0f;
        }
    });

    registry.RegisterSetting({
        Settings::Names::AUTOSAVE_INTERVAL,
        Settings::Categories::GAMEPLAY,
        SettingType::Integer,
        Settings::Defaults::AUTOSAVE_INTERVAL,
        [this](const std::any& value) {
            auto interval = std::any_cast<unsigned int>(value);
            if (m_saveManager) {
                m_saveManager->SetAutosaveInterval(interval);
            }
        },
        [](const std::any& value) {
            auto interval = std::any_cast<unsigned int>(value);
            return interval >= 1 && interval <= 30;
        }
    });
}