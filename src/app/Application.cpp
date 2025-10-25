#include "Application.h"
#include "Constants.h"
#include "Logger.h"
#include "app/Game.h"
#include "app/GameState.h"
#include "components/LineComponents.h"
#include "core/PerfTimer.h"
#include "event/InputEvents.h"
#include "event/UIEvents.h"
#include "input/InputHandler.h"
#include "render/Renderer.h"
#include "systems/gameplay/LineCreationSystem.h"
#include "ui/UI.h"
#include "ui/UIManager.h"
#include <SFML/Graphics/Image.hpp>
#include <array>
#include <chrono>
#include <cctype>
#include <ctime>
#include <filesystem>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#include <limits.h>
#include <unistd.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

namespace {
std::filesystem::path getExecutableDirectory() {
#ifdef _WIN32
    std::wstring buffer(MAX_PATH, L'\0');
    DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0 || length >= buffer.size()) {
        throw std::runtime_error("Failed to resolve executable path.");
    }
    buffer.resize(length);
    return std::filesystem::path(buffer).parent_path();
#elif defined(__APPLE__)
    uint32_t size = 0;
    if (_NSGetExecutablePath(nullptr, &size) != -1) {
        throw std::runtime_error("Failed to determine executable path size.");
    }
    std::string buffer(size, '\0');
    if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
        throw std::runtime_error("Failed to resolve executable path.");
    }
    return std::filesystem::canonical(buffer.c_str()).parent_path();
#else
    std::array<char, PATH_MAX> buffer{};
    ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (length == -1) {
        throw std::runtime_error("Failed to resolve executable path.");
    }
    buffer[static_cast<std::size_t>(length)] = '\0';
    return std::filesystem::path(buffer.data()).parent_path();
#endif
}

sf::Image createAppIcon() {
    constexpr unsigned int iconSize = 128;
    constexpr unsigned int margin = iconSize / 8;
    constexpr unsigned int barThickness = iconSize / 10;

    sf::Image icon({iconSize, iconSize}, sf::Color(10, 14, 22));

    const sf::Color accent(255, 184, 0);
    const unsigned int stemXStart = (iconSize / 2) - (barThickness / 2);
    const unsigned int stemXEnd = stemXStart + barThickness;

    for (unsigned int y = margin; y < margin + barThickness; ++y) {
        for (unsigned int x = margin; x < iconSize - margin; ++x) {
            icon.setPixel({x, y}, accent);
        }
    }

    for (unsigned int y = margin; y < iconSize - margin; ++y) {
        for (unsigned int x = stemXStart; x < stemXEnd; ++x) {
            icon.setPixel({x, y}, accent);
        }
    }

    return icon;
}
} // namespace

Application::Application()
    : _window(sf::VideoMode({Constants::WINDOW_WIDTH, Constants::WINDOW_HEIGHT}),
              Constants::windowTitle(), sf::Style::Default, sf::State::Windowed,
              sf::ContextSettings{0u, 0u, 0u}), // Changed from 16u to 0u
      _colorManager(_eventBus) {
    LOG_INFO("Application", "Application creation started.");
    try {
        const auto exeDir = getExecutableDirectory();
        std::filesystem::current_path(exeDir);
        LOG_INFO("Application", "Working directory set to %s.", exeDir.string().c_str());
    } catch (const std::exception &e) {
        LOG_WARN("Application", "Unable to set working directory: %s", e.what());
    }
    {
        sf::Image icon = createAppIcon();
        const auto size = icon.getSize();
        if (size.x > 0 && size.y > 0) {
            _window.setIcon(icon);
            LOG_INFO("Application", "Window icon applied.");
        } else {
            LOG_WARN("Application", "Generated icon has invalid size, skipping icon setup.");
        }
    }
    try {
        unsigned int numThreads = std::thread::hardware_concurrency();
        _threadPool = std::make_unique<ThreadPool>(numThreads > 0 ? numThreads : 1);
        LOG_DEBUG("Application", "ThreadPool created with %u threads.", numThreads);

        _renderer = std::make_unique<Renderer>(_colorManager, _window);
        _renderer->initialize();

        _game = std::make_unique<Game>(*_renderer, *_threadPool, _eventBus, _colorManager);

        _renderer->connectToEventBus(_eventBus);

        _ui = std::make_unique<UI>(_window, _game->getLoadingState());
        _ui->initialize();
        _ui->setStartNewGameCallback(
            [this](const UI::NewGameOptions &options) { handleStartNewGame(options); });
        _ui->setLoadGameCallback(
            [this](const std::filesystem::path &path) { handleLoadGame(path); });
        _ui->setQuitCallback([this]() { _game->getGameState().currentAppState = AppState::QUITTING; });
        _ui->setSaveGameCallback([this]() { handleSaveGame(); });
        _ui->setResumeCallback([this]() { handleResumeGame(); });

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

        struct LineStateInfo {
            size_t numStationsInActiveLine = 0;
            size_t numPointsInActiveLine = 0;
            std::optional<float> currentSegmentGrade;
            bool currentSegmentExceedsGrade = false;
        };

        auto gatherLineState = [this]() -> LineStateInfo {
            LineStateInfo info;

            if (_game->getRegistry().ctx().contains<ActiveLine>()) {
                const auto &activeLine = _game->getRegistry().ctx().get<ActiveLine>();
                info.numPointsInActiveLine = activeLine.points.size();
                info.numStationsInActiveLine =
                    std::count_if(activeLine.points.begin(), activeLine.points.end(),
                                  [](const LinePoint &p) { return p.type == LinePointType::STOP; });
            }

            if (_game->getRegistry().ctx().contains<LinePreview>()) {
                const auto &preview = _game->getRegistry().ctx().get<LinePreview>();
                if (preview.currentSegmentGrade.has_value()) {
                    info.currentSegmentGrade = preview.currentSegmentGrade;
                    info.currentSegmentExceedsGrade = preview.currentSegmentExceedsGrade;
                }
            }

            return info;
        };

        switch (appState) {
        case AppState::MAIN_MENU: {
            _ui->update(frameTime, appState);
            renderLoad();
            break;
        }
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
            const LineStateInfo lineState = gatherLineState();

            _ui->update(frameTime, appState);
            _uiManager->draw(frameTime, lineState.numStationsInActiveLine,
                             lineState.numPointsInActiveLine, lineState.currentSegmentGrade,
                             lineState.currentSegmentExceedsGrade);
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
        case AppState::PAUSED: {
            _timeAccumulator = sf::Time::Zero;

            const LineStateInfo lineState = gatherLineState();
            _ui->update(frameTime, appState);
            if (_ui->consumeBackToMenuRequest()) {
                handleBackToMenu();
                renderLoad();
                break;
            }
            _uiManager->draw(frameTime, lineState.numStationsInActiveLine,
                             lineState.numPointsInActiveLine, lineState.currentSegmentGrade,
                             lineState.currentSegmentExceedsGrade);
            render(0.0f);
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
            bool suppressGameInput = false;

            if (currentEvent.is<sf::Event::FocusLost>()) {
                _isWindowFocused = false;
                _game->getInputHandler().setWindowFocus(false);
            } else if (currentEvent.is<sf::Event::FocusGained>()) {
                _isWindowFocused = true;
                _game->getInputHandler().setWindowFocus(true);
            }

            if (const auto *keyPressed = currentEvent.getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::Escape) {
                    auto &state = _game->getGameState().currentAppState;
                    suppressGameInput = true;
                    if (state == AppState::PLAYING) {
                        state = AppState::PAUSED;
                        LOG_INFO("Application", "Pause menu opened via Escape key.");
                        _timeAccumulator = sf::Time::Zero;
                        _deltaClock.restart();
                    } else if (state == AppState::PAUSED) {
                        LOG_INFO("Application", "Resume requested via Escape key.");
                        handleResumeGame();
                    }
                }
            }

            _ui->processEvent(currentEvent);
            if (!suppressGameInput
                && _game->getGameState().currentAppState == AppState::PLAYING) {
                _game->getInputHandler().handleGameEvent(currentEvent, _window);
            }
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

void Application::handleStartNewGame(const UI::NewGameOptions &options) {
    auto &gameState = _game->getGameState();
    gameState.worldName = options.worldName;
    gameState.worldType = options.worldType;
    gameState.gameMode = options.gameMode;
    gameState.currentInteractionMode = InteractionMode::SELECT;
    gameState.selectedEntity.reset();
    gameState.passengerOriginStation.reset();
    gameState.timeMultiplier = 1.0f;
    gameState.preEditTimeMultiplier = 1.0f;
    gameState.totalElapsedTime = sf::Time::Zero;
    gameState.elevationChecksEnabled = true;
    gameState.currentAppState = AppState::LOADING;

    _timeAccumulator = sf::Time::Zero;
    _deltaClock.restart();
    _game->startLoading();
}

void Application::handleLoadGame(const std::filesystem::path &path) {
    auto &gameState = _game->getGameState();
    auto &loadingState = _game->getLoadingState();

    loadingState.message = "Loading saved game...";
    loadingState.progress = 0.0f;
    loadingState.showOverlay = false;

    gameState.currentAppState = AppState::LOADING;

    _eventBus.enqueue<LoadGameRequestEvent>({path.string()});
    _eventBus.update();

    gameState.currentAppState = AppState::PLAYING;
    const std::string inferredName = path.stem().string();
    if (gameState.worldName.empty() || gameState.worldName == "Loaded World") {
        gameState.worldName = inferredName;
    }

    _timeAccumulator = sf::Time::Zero;
    _deltaClock.restart();
}

void Application::handleSaveGame() {
    const std::filesystem::path savePath = generateSaveFilePath();
    LOG_INFO("Application", "Saving game to %s", savePath.string().c_str());

    _eventBus.enqueue<SaveGameRequestEvent>({savePath.string()});
    _eventBus.update();
}

void Application::handleResumeGame() {
    auto &gameState = _game->getGameState();
    if (gameState.currentAppState == AppState::PAUSED) {
        gameState.currentAppState = AppState::PLAYING;
        _timeAccumulator = sf::Time::Zero;
        _deltaClock.restart();
    }
}

void Application::handleBackToMenu() {
    auto &gameState = _game->getGameState();
    LOG_INFO("Application", "Returning to main menu from pause menu.");
    gameState.currentAppState = AppState::MAIN_MENU;
    gameState.currentInteractionMode = InteractionMode::SELECT;
    gameState.selectedEntity.reset();
    gameState.passengerOriginStation.reset();
    gameState.timeMultiplier = 1.0f;
    gameState.preEditTimeMultiplier = 1.0f;
    gameState.totalElapsedTime = sf::Time::Zero;
    _game->getLoadingState().showOverlay = false;
    _timeAccumulator = sf::Time::Zero;
    _deltaClock.restart();
}

std::filesystem::path Application::generateSaveFilePath() {
    auto sanitize = [](const std::string &name) {
        std::string result;
        result.reserve(name.size());
        for (char ch : name) {
            unsigned char c = static_cast<unsigned char>(ch);
            if (std::isalnum(c) || ch == '_' || ch == '-') {
                result.push_back(ch);
            } else if (std::isspace(c)) {
                result.push_back('_');
            }
        }
        return result;
    };

    const std::filesystem::path savesDir = std::filesystem::current_path() / "saves";
    std::error_code dirError;
    std::filesystem::create_directories(savesDir, dirError);

    std::string baseName = sanitize(_game->getGameState().worldName);
    if (baseName.empty()) {
        baseName = "save";
    }

    using std::chrono::system_clock;
    const auto now = system_clock::now();
    const std::time_t nowTime = system_clock::to_time_t(now);
    std::tm timeInfo{};
#if defined(_WIN32)
    localtime_s(&timeInfo, &nowTime);
#else
    localtime_r(&nowTime, &timeInfo);
#endif

    std::ostringstream nameBuilder;
    nameBuilder << baseName << "_" << std::put_time(&timeInfo, "%Y%m%d_%H%M%S");
    std::string filename = nameBuilder.str();

    std::filesystem::path savePath = savesDir / (filename + ".json");
    int counter = 1;
    while (std::filesystem::exists(savePath)) {
        savePath = savesDir / (filename + "_" + std::to_string(counter) + ".json");
        ++counter;
    }

    return savePath;
}
