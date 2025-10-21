#include "Application.h"
#include "Constants.h"
#include "Logger.h"
#include "app/Game.h"
#include "app/GameState.h"
#include "components/LineComponents.h"
#include "core/PerfTimer.h"
#include "event/InputEvents.h"
#include "input/InputHandler.h"
#include "render/Renderer.h"
#include "systems/gameplay/LineCreationSystem.h"
#include "ui/UI.h"
#include "ui/UIManager.h"
#include <optional>
#include <thread>

Application::Application()
    : _window(sf::VideoMode({Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT}),
              Constants::WINDOW_TITLE, sf::Style::Default, sf::State::Windowed,
              sf::ContextSettings{0u, 0u, 0u}), // Changed from 16u to 0u
      _colorManager(_eventBus) {
    LOG_INFO("Application", "Application creation started.");
    try {
        unsigned int numThreads = std::thread::hardware_concurrency();
        _threadPool = std::make_unique<ThreadPool>(numThreads > 0 ? numThreads : 1);
        LOG_DEBUG("Application", "ThreadPool created with %u threads.", numThreads);

        _renderer = std::make_unique<Renderer>(_colorManager, _window);
        _renderer->initialize();

        _game = std::make_unique<Game>(*_renderer, *_threadPool, _eventBus, _colorManager);
        _game->startLoading();

        _renderer->connectToEventBus(_eventBus);

        _ui = std::make_unique<UI>(_window, _game->getLoadingState());
        _ui->initialize();

        _uiManager = std::make_unique<UIManager>(
            _game->getRegistry(), _eventBus, _game->getWorldGenSystem(),
            _renderer->getTerrainRenderSystem(), _game->getPerformanceMonitor(), _game->getCamera(),
            _game->getGameState(), _colorManager, _window, _game->getCityPlacementSystem());

    } catch (const std::exception &e) {
        LOG_FATAL("Application", "Failed during initialization: %s", e.what());
        throw;
    }
    LOG_INFO("Application", "Application created successfully.");
}

Application::~Application() = default;

void Application::run() {
    LOG_INFO("Application", "Starting main loop.");
    while (_renderer->isWindowOpen()) {
        sf::Time frameTime = _deltaClock.restart();
        if (frameTime > sf::milliseconds(250)) {
            frameTime = sf::milliseconds(250);
        }
        _timeAccumulator += frameTime;

        processEvents();

        const auto appState = _game->getGameState().currentAppState;

        switch (appState) {
        case AppState::LOADING: {
            _ui->update(frameTime, appState);
            if (_game->getLoadingFuture().wait_for(std::chrono::seconds(0))
                == std::future_status::ready) {
                _game->getGameState().currentAppState = AppState::PLAYING;
                LOG_INFO("Application", "Loading complete, switching to PLAYING state.");

                _timeAccumulator = sf::Time::Zero;
                _deltaClock.restart();
            }
            renderLoad();
            break;
        }
        case AppState::PLAYING: {
            size_t numStationsInActiveLine = 0;
            size_t numPointsInActiveLine = 0;
            std::optional<float> currentSegmentGrade;
            bool currentSegmentExceedsGrade = false;
            if (_game->getRegistry().ctx().contains<ActiveLine>()) {
                const auto &activeLine = _game->getRegistry().ctx().get<ActiveLine>();
                numPointsInActiveLine = activeLine.points.size();
                numStationsInActiveLine =
                    std::count_if(activeLine.points.begin(), activeLine.points.end(),
                                  [](const LinePoint &p) { return p.type == LinePointType::STOP; });
            }

            if (_game->getRegistry().ctx().contains<LinePreview>()) {
                const auto &preview = _game->getRegistry().ctx().get<LinePreview>();
                if (preview.currentSegmentGrade.has_value()) {
                    currentSegmentGrade = preview.currentSegmentGrade;
                    currentSegmentExceedsGrade = preview.currentSegmentExceedsGrade;
                }
            }

            bool isLineSelected = false;
            if (const auto &selectedEntity = _game->getGameState().selectedEntity;
                selectedEntity.has_value()) {
                if (_game->getRegistry().all_of<LineComponent>(selectedEntity.value())) {
                    isLineSelected = true;
                }
            }

            _ui->update(frameTime, appState);
            _uiManager->draw(frameTime, numStationsInActiveLine, numPointsInActiveLine,
                             currentSegmentGrade, currentSegmentExceedsGrade);
            update(TimePerFrame);

            while (_timeAccumulator >= TimePerFrame) {
                _timeAccumulator -= TimePerFrame;
                if (_game->getGameState().timeMultiplier > 0.0f) {
                    sf::Time scaledTimePerFrame =
                        TimePerFrame * _game->getGameState().timeMultiplier;
                    _game->updateSimulation(scaledTimePerFrame);
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
    while (auto optEvent = _window.pollEvent()) {
        if (optEvent) {
            const sf::Event &currentEvent = *optEvent;

            if (currentEvent.is<sf::Event::FocusLost>()) {
                _isWindowFocused = false;
                _game->getInputHandler().setWindowFocus(false);
            } else if (currentEvent.is<sf::Event::FocusGained>()) {
                _isWindowFocused = true;
                _game->getInputHandler().setWindowFocus(true);
            }

            _ui->processEvent(currentEvent);
            _game->getInputHandler().handleGameEvent(currentEvent, _window);
        }
    }
}

void Application::update(sf::Time dt) {
    PerfTimer timer("Application::update", _game->getPerformanceMonitor());

    _eventBus.update();
    _game->getInputHandler().update(dt);
    _game->getGameState().totalElapsedTime += dt;
    _game->update(dt, *_ui);
}

void Application::render(float interpolation) {
    PerfTimer timer("Application::render", _game->getPerformanceMonitor());

    _renderer->clear(); // Clear the main window

    const auto &worldGen = _game->getWorldGenSystem();
    auto &passengerSpawnAnimationSystem = _game->getPassengerSpawnAnimationSystem();

    _renderer->renderFrame(_game->getRegistry(), _game->getGameState(),
                           _game->getCamera().getView(), worldGen, passengerSpawnAnimationSystem,
                           interpolation);
    _ui->renderFrame();
    _renderer->displayFrame();
}

void Application::renderLoad() {
    _renderer->clear();
    _ui->renderFrame();
    _renderer->displayFrame();
}
