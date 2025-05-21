#include "Game.h"
#include "Renderer.h"
#include "InputHandler.h"
#include "Logger.h"
#include <cstdlib> // For exit()
#include <memory>
#include <string> // Required for std::to_string in EntityFactory if not included there
// SFML includes for specific types like sf::Time, sf::Clock are in Game.h

Game::Game()
    : m_entityFactory(registry) {
    LOG_INFO("Game", "Game instance creating.");
    LOG_INFO("Game", "Game instance created successfully.");
}

void Game::init() {
    LOG_INFO("Game", "Game initialization started.");
    LOG_INFO("Main", "Logger initialized. Enabled: %s, MinLevel: %s", Logging::Logger::getInstance().isLoggingEnabled() ? "true" : "false", Logging::Logger::getInstance().logLevelToString(Logging::Logger::getInstance().getMinLogLevel()));
    LOG_INFO("Main", "Default global log delay set to: %ums", Logging::Logger::getInstance().getLogDelay());
    LOG_INFO("Main", "Application starting.");
    try {
        m_renderer = std::make_unique<Renderer>();
        if (!m_renderer) {
            LOG_FATAL("Game", "Failed to create Renderer instance (m_renderer is null).");
            exit(EXIT_FAILURE);
        }
        m_renderer->init();

        m_ui = std::make_unique<UI>(m_renderer->getWindow());
        if (!m_ui) {
            LOG_FATAL("Game", "Failed to create UI instance (m_ui is null).");
            exit(EXIT_FAILURE);
        }
        m_ui->init();

        m_inputHandler = std::make_unique<InputHandler>();
        if (!m_inputHandler) {
            LOG_FATAL("Game", "Failed to create InputHandler instance (m_inputHandler is null).");
            exit(EXIT_FAILURE);
        }

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
                    sf::View& view = camera.getViewToModify();
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
                if (m_ui->getInteractionMode() == InteractionMode::StationPlacement) {
                    LOG_DEBUG("Game", "Processing TryPlaceStation command at (%.1f, %.1f)", command.data.worldPosition.x, command.data.worldPosition.y);
                    int nextStationID = registry.alive() ? static_cast<int>(registry.size()) : 0;
                    m_entityFactory.createStation(command.data.worldPosition, "New Station " + std::to_string(nextStationID));
                }
                break;
            case InputEventType::None:
            default:
                break;
        }
    }
    m_inputHandler->clearCommands();
}


void Game::run() {
    LOG_INFO("Game", "Starting game loop.");
    while (m_renderer->getWindow().isOpen()) {
        sf::Time dt = deltaClock.restart();
        LOG_TRACE("Game", "Delta time: %f seconds", dt.asSeconds());

        while (auto optEvent = m_renderer->getWindow().pollEvent()) {
            if (optEvent) {
                const sf::Event& currentEvent = *optEvent;
                m_ui->processEvent(currentEvent);
                m_inputHandler->handleGameEvent(currentEvent, m_ui->getInteractionMode(), camera, m_renderer->getWindow());
            }
        }
        
        m_inputHandler->update(dt);

        processInputCommands();

        m_ui->update(dt);

        update(dt);
        LOG_TRACE("Game", "Game logic updated.");

        m_renderer->render(registry, camera.getView(), dt);
        LOG_TRACE("Game", "Frame rendered.");

        m_ui->render();

        m_renderer->display();
    }
    LOG_INFO("Game", "Game loop ended.");
    m_renderer->cleanup();
    m_ui->cleanup();
    LOG_INFO("Game", "Renderer and UI cleaned up.");
}

void Game::update(sf::Time dt) {
}

Game::~Game() {
    LOG_INFO("Main", "Application shutting down.");
}