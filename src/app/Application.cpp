
#include "Application.h"
#include "Constants.h"
#include "Logger.h"
#include "app/GameState.h"
#include "core/PerfTimer.h"
#include "core/ServiceLocator.h"
#include "event/InputEvents.h"
#include "input/InputHandler.h"
#include "systems/gameplay/LineCreationSystem.h"

#include <stdexcept>
#include <thread>

Application::Application() : _colorManager(_eventBus) {
    LOG_INFO("Application", "Application creation started.");
    try {
        unsigned int numThreads = std::thread::hardware_concurrency();
        _threadPool = std::make_unique<ThreadPool>(numThreads > 0 ? numThreads : 1);
        LOG_INFO("Application", "ThreadPool created with %u threads.", numThreads);

        LOG_INFO("Application", "Creating Renderer object...");
        _renderer = std::make_unique<Renderer>(_colorManager);
        LOG_INFO("Application", "Renderer object created.");
        _renderer->initialize();
        LOG_INFO("Application", "Renderer initialized.");

        LOG_INFO("Application", "Creating Game object...");
        _game = std::make_unique<Game>(*_renderer, *_threadPool, _eventBus, _colorManager);
        LOG_INFO("Application", "Game object created.");
        _game->startLoading();

        _renderer->connectToEventBus(_eventBus);

        _ui = std::make_unique<UI>(_renderer->getWindowInstance(),
                                   _renderer->getTerrainRenderSystem(), 
                                   _game->getServiceLocator());
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

        const auto appState = _game->getGameState().currentAppState;

        switch (appState) {
        case AppState::LOADING: {
            _ui->update(frameTime, 0);
            if (_game->getLoadingFuture().wait_for(std::chrono::seconds(0))
                == std::future_status::ready) {
                _game->getGameState().currentAppState = AppState::PLAYING;
                LOG_INFO("Application", "Loading complete, switching to PLAYING state.");
            }
            renderLoad();
            break;
        }
        case AppState::PLAYING: {
            size_t numStationsInActiveLine = 0;
            if (auto lineCreationSystem =
                    _game->getSystemManager().getSystem<LineCreationSystem>()) {
                lineCreationSystem->getActiveLineStations(
                    [&numStationsInActiveLine](entt::entity) { numStationsInActiveLine++; });
            }

            _ui->update(frameTime, numStationsInActiveLine);

            if (_isWindowFocused) {
                while (_timeAccumulator >= TimePerFrame) {
                    _timeAccumulator -= TimePerFrame;
                    sf::Time scaledTimePerFrame = TimePerFrame * _game->getGameState().timeMultiplier;
                    update(scaledTimePerFrame);
                }
            }

            const float interpolation = _timeAccumulator.asSeconds() / TimePerFrame.asSeconds();
            render(interpolation);
            break;
        }
        case AppState::QUITTING: {
            _eventBus.trigger<WindowCloseEvent>();
            break;
        }
        }
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

    _eventBus.update();
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

void Application::renderLoad() {
    _renderer->clear();
    _ui->renderFrame();
    _renderer->displayFrame();
}