#pragma once

#include "app/GameState.h"
#include "app/LoadingState.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Window/Event.hpp>
#include <array>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

class UI {
public:
    struct NewGameOptions {
        std::string worldName;
        WorldType worldType;
        GameMode gameMode;
    };

    using StartNewGameCallback = std::function<void(const NewGameOptions &)>;
    using LoadGameCallback = std::function<void(const std::filesystem::path &)>;
    using QuitCallback = std::function<void()>;
    using SaveGameCallback = std::function<void()>;
    using ResumeCallback = std::function<void()>;

    UI(sf::RenderWindow &window, LoadingState &loadingState);
    ~UI();
    void initialize();
    void processEvent(const sf::Event &event);
    void update(sf::Time deltaTime, AppState appState);
    void renderFrame();
    void cleanupResources();

    void setStartNewGameCallback(StartNewGameCallback callback);
    void setLoadGameCallback(LoadGameCallback callback);
    void setQuitCallback(QuitCallback callback);
    void setSaveGameCallback(SaveGameCallback callback);
    void setResumeCallback(ResumeCallback callback);
    bool consumeBackToMenuRequest();

private:
    enum class MenuScreen { Main, NewGame, LoadGame };

    void drawLoadingScreen();
    void drawMainMenu();
    void drawMainMenuHome();
    void drawMainMenuOverlays();
    void drawNewGameScreen();
    void drawLoadGameScreen();
    void drawPauseMenu();
    void drawRegenerationModal();
    void refreshSaveEntries();

    sf::RenderWindow &_window;
    LoadingState &_loadingState;
    StartNewGameCallback _startNewGameCallback;
    LoadGameCallback _loadGameCallback;
    QuitCallback _quitCallback;
    SaveGameCallback _saveGameCallback;
    ResumeCallback _resumeCallback;
    bool _regenerationModalOpen = false;
    MenuScreen _currentMenuScreen = MenuScreen::Main;
    std::filesystem::path _saveDirectory;
    std::array<char, 128> _newGameNameBuffer{};
    int _worldTypeIndex = 0;
    int _gameModeIndex = 1;
    std::string _newGameError;

    struct SaveEntry {
        std::string displayName;
        std::filesystem::path fullPath;
    };

    std::vector<SaveEntry> _saveEntries;
    int _selectedSaveIndex = -1;
    std::string _loadGameError;
    bool _showPauseConfirm = false;
    bool _backToMenuRequested = false;
};
