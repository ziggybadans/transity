
#include "Application.h"
#include "Constants.h"
#include "Logger.h"
#include "core/PerfTimer.h"
#include "core/ServiceLocator.h"
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

        _ui = std::make_unique<UI>(_renderer->getWindowInstance(),
                                   _renderer->getTerrainRenderSystem(), _game->getServiceLocator());
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
        _ui->update(frameTime, 0);

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

            _ui->processEvent(currentEvent);
            if (_isWindowFocused) {

                _game->getInputHandler().handleGameEvent(currentEvent,
                                                         _renderer->getWindowInstance());
            }
        }
    }
}

void Application::update(sf::Time dt) {
    PerfTimer timer("Application::update", _game->getServiceLocator());

    _game->getInputHandler().update(dt);
    _game->update(dt, *_ui);
}

void Application::render(float interpolation) {
    PerfTimer timer("Application::render", _game->getServiceLocator());

    const auto &worldGen = _game->getWorldGenSystem();

    _renderer->renderFrame(_game->getRegistry(), _game->getCamera().getView(), worldGen,
                           interpolation);
    _ui->renderFrame();
    _renderer->displayFrame();
}