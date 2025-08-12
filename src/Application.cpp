
#include "Application.h"
#include "Logger.h"
#include "core/Constants.h"
#include "input/InputHandler.h"
#include <stdexcept>
#include <thread>

Application::Application() {
    LOG_INFO("Application", "Application creation started.");
    try {
        unsigned int numThreads = std::thread::hardware_concurrency();
        _threadPool = std::make_unique<ThreadPool>(numThreads > 0 ? numThreads : 1);
        LOG_INFO("Application", "ThreadPool created with %u threads.", numThreads);

        _renderer = std::make_unique<Renderer>();
        _renderer->initialize();

        _game = std::make_unique<Game>(*_renderer, *_threadPool);

        _renderer->connectToEventBus(_game->getEventBus());

        _game->init();

        _ui = std::make_unique<UI>(_renderer->getWindowInstance(), _game->getRegistry(),
                                   &_game->getWorldGenerationSystem(),
                                   &_renderer->getTerrainRenderSystem(), _game->getGameState(),
                                   _game->getEventBus(), _game->getCamera());
        _ui->initialize();

    } catch (const std::exception &e) {
        LOG_FATAL("Application", "Failed during initialization: %s", e.what());
        throw;
    }
    LOG_INFO("Application", "Application created successfully.");
}

void Application::run() {
    LOG_INFO("Application", "Starting main loop.");
    while (_renderer->isWindowOpen()) {
        sf::Time frameTime = _deltaClock.restart();
        _timeAccumulator += frameTime;

        processEvents();

        // Update UI with real frame time once per frame
        _ui->update(frameTime, _game->getActiveStationCount());

        // Perform fixed updates if focused
        if (_isWindowFocused) {
            while (_timeAccumulator >= TimePerFrame) {
                _timeAccumulator -= TimePerFrame;
                update(TimePerFrame);
            }
        }

        // Calculate interpolation for smooth rendering
        const float interpolation = _timeAccumulator.asSeconds() / TimePerFrame.asSeconds();
        render(interpolation);
    }
    LOG_INFO("Application", "Main loop ended.");
    _renderer->cleanupResources();
    _ui->cleanupResources();
}

void Application::processEvents() {
    while (auto optEvent = _renderer->getWindowInstance().pollEvent()) {
        if (optEvent) {
            const sf::Event &currentEvent = *optEvent;

            if (currentEvent.is<sf::Event::FocusLost>()) {
                _isWindowFocused = false;
            } else if (currentEvent.is<sf::Event::FocusGained>()) {
                _isWindowFocused = true;
            }

            if (const auto *resizedEvent = currentEvent.getIf<sf::Event::Resized>()) {
                _game->onWindowResize(resizedEvent->size.x, resizedEvent->size.y);
            }

            _ui->processEvent(currentEvent);
            if (_isWindowFocused) {

                _game->getInputHandler().handleGameEvent(currentEvent,
                                                         _renderer->getWindowInstance());
            }
        }
    }
}

void Application::update(sf::Time dt) {
    _game->getInputHandler().update(dt);
    _game->update(dt, *_ui);
}


void Application::render(float interpolation) {
    // Pass interpolation to the renderer
    _renderer->renderFrame(_game->getRegistry(), _game->getCamera().getView(), interpolation);
    _ui->renderFrame();
    _renderer->displayFrame();
}