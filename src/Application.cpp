// src/Application.cpp
#include "Application.h"
#include "Logger.h"
#include "core/Constants.h"
#include "input/InputHandler.h"
#include <stdexcept>

Application::Application() {
    LOG_INFO("Application", "Application creation started.");
    try {
        _renderer = std::make_unique<Renderer>();
        _renderer->initialize();

        // Game is created, and it now creates its own InputHandler
        _game = std::make_unique<Game>(*_renderer);

        _renderer->connectToEventBus(_game->getEventBus());
        
        // InputHandler is no longer created here
        
        _game->init();

        _ui = std::make_unique<UI>(_renderer->getWindowInstance(), &_game->getWorldGenerationSystem(), _game->getGameState(), _game->getEventBus());
        _ui->initialize();

    } catch (const std::exception& e) {
        LOG_FATAL("Application", "Failed during initialization: %s", e.what());
        throw;
    }
    LOG_INFO("Application", "Application created successfully.");
}

void Application::run() {
    LOG_INFO("Application", "Starting main loop.");
    while (_renderer->isWindowOpen()) {
        sf::Time dt = _deltaClock.restart();

        processEvents();
        
        if (_isWindowFocused) {
            update(dt);
        }
        
        render(dt);
    }
    LOG_INFO("Application", "Main loop ended.");
    _renderer->cleanupResources();
    _ui->cleanupResources();
}

void Application::processEvents() {
    while (auto optEvent = _renderer->getWindowInstance().pollEvent()) {
        if (optEvent) {
            const sf::Event& currentEvent = *optEvent;

            if (currentEvent.is<sf::Event::FocusLost>()) {
                _isWindowFocused = false;
            } else if (currentEvent.is<sf::Event::FocusGained>()) {
                _isWindowFocused = true;
            }

            if (const auto* resizedEvent = currentEvent.getIf<sf::Event::Resized>()) {
                _game->onWindowResize(resizedEvent->size.x, resizedEvent->size.y);
            }

            _ui->processEvent(currentEvent);
            if (_isWindowFocused) {
                // Get InputHandler from Game and call the simplified method
                _game->getInputHandler().handleGameEvent(currentEvent, _renderer->getWindowInstance());
            }
        }
    }
}

void Application::update(sf::Time dt) {
    // Get InputHandler from Game and call the simplified method
    _game->getInputHandler().update(dt);
    _game->update(dt, *_ui);
    _ui->update(dt, _game->getActiveStationCount());
}

void Application::render(sf::Time dt) {
    _renderer->renderFrame(_game->getRegistry(), _game->getCamera().getView(), dt, _ui->getVisualizeNoiseState());
    _ui->renderFrame();
    _renderer->displayFrame();
}
