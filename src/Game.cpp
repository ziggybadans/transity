#include "Game.h"
#include "Renderer.h"
#include "InputHandler.h"
#include "Logger.h"
#include <SFML/Window/Event.hpp> // Added for sf::Event
#include <cstdlib> // For exit()
#include <memory>
#include <string> // Required for std::to_string in EntityFactory if not included there
// SFML includes for specific types like sf::Time, sf::Clock are in Game.h

Game::Game()
    : m_entityFactory(registry) { // m_currentInteractionMode removed
    LOG_INFO("Game", "Game instance creating.");
    LOG_INFO("Game", "Game instance created successfully.");
}

void Game::init() {
    LOG_INFO("Game", "Game initialization started.");
    LOG_INFO("Main", "Logger initialized. Enabled: %s, MinLevel: %s", Logging::Logger::getInstance().isLoggingEnabled() ? "true" : "false", Logging::Logger::getInstance().logLevelToString(Logging::Logger::getInstance().getMinLogLevel()));
    LOG_INFO("Main", "Default global log delay set to: %ums", Logging::Logger::getInstance().getLogDelay());
    LOG_INFO("Main", "Application starting.");
    try {
        m_renderer = std::make_unique<Renderer>(); // Renderer now default constructs and manages its window
        if (!m_renderer) {
            LOG_FATAL("Game", "Failed to create Renderer instance (m_renderer is null).");
            exit(EXIT_FAILURE);
        }
        m_renderer->init(); // Renderer initializes its own window and ImGui

        m_inputHandler = std::make_unique<InputHandler>(m_renderer->getWindow(), camera); // EntityFactory removed
        if (!m_inputHandler) {
            LOG_FATAL("Game", "Failed to create InputHandler instance (m_inputHandler is null).");
            exit(EXIT_FAILURE);
        }
        
        m_uiManager = std::make_unique<UIManager>();
        if (!m_uiManager) {
            LOG_FATAL("Game", "Failed to create UIManager instance (m_uiManager is null).");
            exit(EXIT_FAILURE);
        }
        m_uiManager->init(m_renderer->getWindow());

    } catch (const std::bad_alloc& e) {
        LOG_FATAL("Game", "Failed to allocate memory for core systems: %s", e.what());
        exit(EXIT_FAILURE);
    } catch (const std::exception& e) {
        LOG_FATAL("Game", "An unexpected exception occurred during core system creation: %s", e.what());
        exit(EXIT_FAILURE);
    } catch (...) {
        LOG_FATAL("Game", "An unknown exception occurred during core system creation.");
        exit(EXIT_FAILURE);
    }

    sf::Vector2f landCenter = m_renderer->getLandCenter();
    sf::Vector2f landSize = m_renderer->getLandSize();
    camera.setInitialView(m_renderer->getWindow(), landCenter, landSize);
    Logging::Logger::getInstance().setLogLevelDelay(Logging::LogLevel::TRACE, 2000);
    LOG_INFO("Main", "TRACE log delay set to: %ums", Logging::Logger::getInstance().getLogLevelDelay(Logging::LogLevel::TRACE));
    LOG_INFO("Game", "Game initialization completed.");
}

void Game::processInputCommands() {
    const auto& commands = m_inputHandler->getCommands();
    for (const auto& command : commands) {
        switch (command.type) {
            case InputEventType::WindowClose:
                LOG_INFO("Game", "Processing WindowClose command.");
                m_renderer->getWindow().close();
                break;
            case InputEventType::CameraZoom:
                {
                    LOG_DEBUG("Game", "Processing CameraZoom command with delta: %.2f", command.data.zoomDelta);
                    // The original InputHandler logic for zoom involved maintaining focus.
                    // This needs to be replicated here.
                    sf::View& view = camera.getViewToModify(); // Assuming Camera has getViewToModify()
                    sf::Vector2f worldPosBeforeZoom = m_renderer->getWindow().mapPixelToCoords(command.data.mousePixelPosition, view);
                    camera.zoomView(command.data.zoomDelta);
                    sf::Vector2f worldPosAfterZoom = m_renderer->getWindow().mapPixelToCoords(command.data.mousePixelPosition, view);
                    sf::Vector2f offset = worldPosBeforeZoom - worldPosAfterZoom;
                    camera.moveView(offset);
                    LOG_TRACE("Game", "View moved by (%.1f, %.1f) to maintain zoom focus.", offset.x, offset.y);
                }
                break;
            case InputEventType::CameraPan:
                LOG_DEBUG("Game", "Processing CameraPan command with direction: (%.1f, %.1f)", command.data.panDirection.x, command.data.panDirection.y);
                camera.moveView(command.data.panDirection);
                break;
            case InputEventType::TryPlaceStation:
                if (m_uiManager->getCurrentInteractionMode() == InteractionMode::StationPlacement) {
                    LOG_DEBUG("Game", "Processing TryPlaceStation command at (%.1f, %.1f)", command.data.worldPosition.x, command.data.worldPosition.y);
                    int nextStationID = registry.alive() ? static_cast<int>(registry.size()) : 0;
                    m_entityFactory.createStation(command.data.worldPosition, "New Station " + std::to_string(nextStationID));
                }
                break;
            // Add similar checks for other mode-specific commands like TryStartLine if they exist
            case InputEventType::None:
            default:
                break;
        }
    }
    m_inputHandler->clearCommands();
}


void Game::run() {
    LOG_INFO("Game", "Starting game loop.");
    while (m_inputHandler->isWindowOpen() && m_renderer->getWindow().isOpen()) { // Check both InputHandler's view and actual window
        sf::Time dt = deltaClock.restart();
        LOG_TRACE("Game", "Delta time: %f seconds", dt.asSeconds());

        sf::Event event{}; // Initialize event
        while (m_renderer->getWindow().pollEvent(event)) { // Event loop moved to Game
            m_uiManager->processEvent(event);       // Pass event to UIManager
            m_inputHandler->handleEvent(event);     // Pass event to InputHandler (handleEvent needs to be created)
        }
        
        m_inputHandler->update(dt);      // InputHandler checks for continuous input
        processInputCommands();          // Game processes the commands from InputHandler

        m_uiManager->update();           // Update UIManager (ImGui state)
        // m_currentInteractionMode is now handled by UIManager

        update(dt); // Game-specific update logic
        LOG_TRACE("Game", "Game logic updated.");

        m_renderer->render(registry, camera.getView(), dt); // Render game world
        LOG_TRACE("Game", "Frame rendered.");

        m_uiManager->render();           // Render UI (calls ImGui::SFML::Render)
        // m_renderer->renderImGui(); // This is now done by m_uiManager->render()

        m_renderer->display();
    }
    LOG_INFO("Game", "Game loop ended.");
    
    if (m_uiManager) {
        m_uiManager->shutdown(); // Shutdown UIManager
        LOG_INFO("Game", "UIManager shutdown.");
    }
    if (m_renderer) {
        m_renderer->cleanup(); // Cleanup Renderer (which also shuts down ImGui if it was doing it before)
        LOG_INFO("Game", "Renderer cleaned up.");
    }
}

void Game::update(sf::Time dt) {
    // Camera updates are now driven by commands processed in processInputCommands()
}

Game::~Game() {
    LOG_INFO("Main", "Application shutting down.");
}