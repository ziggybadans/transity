#include "Application.h"
#include "Logger.h"
#include "core/Constants.h"
#include <stdexcept>

Application::Application() {
    LOG_INFO("Application", "Application creation started.");
    try {
        _renderer = std::make_unique<Renderer>();
        _renderer->initialize();

        _inputHandler = std::make_unique<InputHandler>();
        
        _game = std::make_unique<Game>(*_renderer, *_inputHandler);
        _game->init();

        // Correctly pass a pointer to the UI constructor using the address-of operator (&)
        _ui = std::make_unique<UI>(_renderer->getWindowInstance(), &_game->getWorldGenerationSystem());
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

            if (const auto* closedEvent = currentEvent.getIf<sf::Event::Closed>()) {
                _renderer->getWindowInstance().close();
            }

            if (const auto* resizedEvent = currentEvent.getIf<sf::Event::Resized>()) {
                _game->onWindowResize(resizedEvent->size.x, resizedEvent->size.y);
            }

            _ui->processEvent(currentEvent);
            if (_isWindowFocused) {
                _inputHandler->handleGameEvent(currentEvent, _ui->getInteractionMode(), _game->getCamera(), _renderer->getWindowInstance(), _game->getRegistry());
            }
        }
    }
}

void Application::update(sf::Time dt) {
    _inputHandler->update(dt, _game->getCamera());
    _game->update(dt, *_inputHandler, *_ui);
    _ui->update(dt, _game->getActiveStationCount());
}

void Application::render(sf::Time dt) {
    _renderer->renderFrame(_game->getRegistry(), _game->getCamera().getView(), dt, _ui->getVisualizeNoiseState());
    _ui->renderFrame();
    _renderer->displayFrame();
}
