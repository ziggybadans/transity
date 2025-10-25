#include "UI.h"
#include "Constants.h"
#include "Logger.h"
#include "app/LoadingState.h"
#include "imgui-SFML.h"
#include "imgui.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <utility>

UI::UI(sf::RenderWindow &window, LoadingState &loadingState)
    : _window(window), _loadingState(loadingState),
      _saveDirectory(std::filesystem::current_path() / "saves") {
    LOG_DEBUG("UI", "UI instance created.");
    const std::string defaultName = "New World";
    std::fill(_newGameNameBuffer.begin(), _newGameNameBuffer.end(), '\0');
    std::copy_n(defaultName.c_str(),
                std::min(defaultName.size(), _newGameNameBuffer.size() - 1),
                _newGameNameBuffer.begin());
}

UI::~UI() {
    LOG_DEBUG("UI", "UI instance destroyed.");
}

void UI::initialize() {
    LOG_INFO("UI", "Initializing ImGui.");
    ImGui::CreateContext();
    if (!ImGui::SFML::Init(_window)) {
        LOG_FATAL("UI", "Failed to initialize ImGui-SFML");
        exit(EXIT_FAILURE);
    }
    ImGui::StyleColorsDark();
    LOG_INFO("UI", "ImGui initialized successfully.");
}

void UI::processEvent(const sf::Event &sfEvent) {
    ImGui::SFML::ProcessEvent(_window, sfEvent);
}

void UI::update(sf::Time deltaTime, AppState appState) {
    ImGui::SFML::Update(_window, deltaTime);

    if (appState != AppState::MAIN_MENU) {
        _currentMenuScreen = MenuScreen::Main;
    }

    if (appState == AppState::MAIN_MENU) {
        drawMainMenu();
    } else if (appState == AppState::PAUSED) {
        drawPauseMenu();
    } else if (appState == AppState::LOADING) {
        drawLoadingScreen();
    }

    drawRegenerationModal();
}

void UI::renderFrame() {
    ImGui::SFML::Render(_window);
}

void UI::cleanupResources() {
    LOG_INFO("UI", "Shutting down ImGui.");
    ImGui::SFML::Shutdown();
    LOG_INFO("UI", "ImGui shutdown complete.");
}

void UI::setStartNewGameCallback(StartNewGameCallback callback) {
    _startNewGameCallback = std::move(callback);
}

void UI::setLoadGameCallback(LoadGameCallback callback) {
    _loadGameCallback = std::move(callback);
}

void UI::setQuitCallback(QuitCallback callback) {
    _quitCallback = std::move(callback);
}

void UI::setSaveGameCallback(SaveGameCallback callback) {
    _saveGameCallback = std::move(callback);
}

void UI::setResumeCallback(ResumeCallback callback) {
    _resumeCallback = std::move(callback);
}

bool UI::consumeBackToMenuRequest() {
    if (_backToMenuRequested) {
        _backToMenuRequested = false;
        LOG_INFO("UI", "Pause menu back-to-menu request consumed.");
        return true;
    }
    return false;
}

void UI::drawLoadingScreen() {
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    const char *message = _loadingState.message.load();
    float progress = _loadingState.progress.load();

    ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f), ImGuiCond_Always,
                            ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(displaySize.x * 0.4f, 0));

    ImGui::Begin("Loading", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);

    ImGui::Text("%s", message);

    ImGui::Dummy(ImVec2(0.0f, 5.0f));
    ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f), "");

    ImGui::SameLine(0.0f, 0.0f);
    std::string progressText = std::to_string(static_cast<int>(progress * 100.0f)) + "%";
    ImVec2 progressTextSize = ImGui::CalcTextSize(progressText.c_str());
    ImGui::SetCursorPosX((ImGui::GetWindowSize().x - progressTextSize.x) * 0.5f);
    ImGui::Text("%s", progressText.c_str());

    ImGui::Dummy(ImVec2(0.0f, 5.0f));

    ImGui::End();
}

void UI::drawRegenerationModal() {
    const bool overlayActive = _loadingState.showOverlay.load();

    if (overlayActive && !_regenerationModalOpen) {
        ImGui::OpenPopup("Regenerating Entities");
        _regenerationModalOpen = true;
    }

    ImGuiIO &io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                            ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(360.0f, 0.0f), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Regenerating Entities", nullptr,ImGuiWindowFlags_NoSavedSettings
                                   | ImGuiWindowFlags_NoMove)) {
        const char *message = _loadingState.message.load();
        float progress = _loadingState.progress.load();

        ImGui::TextWrapped("%s", message);

        ImGui::Dummy(ImVec2(0.0f, 6.0f));
        ImGui::ProgressBar(progress, ImVec2(-1.0f, 0.0f));
        ImGui::Dummy(ImVec2(0.0f, 6.0f));

        const std::string progressText =
            std::to_string(static_cast<int>(progress * 100.0f + 0.5f)) + "%";
        ImGui::SetCursorPosX(
            std::max(0.0f, (ImGui::GetWindowSize().x - ImGui::CalcTextSize(progressText.c_str()).x) * 0.5f));
        ImGui::TextUnformatted(progressText.c_str());

        if (!overlayActive || progress >= 1.0f) {
            ImGui::CloseCurrentPopup();
            _regenerationModalOpen = false;
            _loadingState.showOverlay = false;
        }

        ImGui::EndPopup();
    } else if (!overlayActive) {
        _regenerationModalOpen = false;
    }
}
void UI::drawMainMenu() {
    drawMainMenuOverlays();
    switch (_currentMenuScreen) {
    case MenuScreen::Main:
        drawMainMenuHome();
        break;
    case MenuScreen::NewGame:
        drawNewGameScreen();
        break;
    case MenuScreen::LoadGame:
        drawLoadGameScreen();
        break;
    }
}

void UI::drawMainMenuOverlays() {
    ImGuiIO &io = ImGui::GetIO();
    const ImGuiWindowFlags overlayFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs
                                          | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings
                                          | ImGuiWindowFlags_AlwaysAutoResize
                                          | ImGuiWindowFlags_NoFocusOnAppearing;

    ImGui::SetNextWindowPos(ImVec2(Constants::UI_WINDOW_PADDING, Constants::UI_WINDOW_PADDING),
                            ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f);
    if (ImGui::Begin("##MainMenuVersionOverlay", nullptr, overlayFlags)) {
        ImGui::TextUnformatted(Constants::versionBanner().c_str());
    }
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(Constants::UI_WINDOW_PADDING,
                                   io.DisplaySize.y - Constants::UI_WINDOW_PADDING),
                            ImGuiCond_Always, ImVec2(0.0f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.0f);
    if (ImGui::Begin("##MainMenuCopyrightOverlay", nullptr, overlayFlags)) {
        ImGui::TextUnformatted(Constants::copyrightNotice().c_str());
    }
    ImGui::End();
}

void UI::drawMainMenuHome() {
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    const ImVec2 windowSize(420.0f, 320.0f);

    ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f), ImGuiCond_Always,
                            ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(windowSize);
    ImGui::Begin("Main Menu", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
                     | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

    const char *title = "Transity";
    ImVec2 titleSize = ImGui::CalcTextSize(title);
    ImGui::SetCursorPosX((ImGui::GetWindowWidth() - titleSize.x) * 0.5f);
    ImGui::TextUnformatted(title);

    ImGui::Dummy(ImVec2(0.0f, 20.0f));

    if (ImGui::Button("New Game", ImVec2(-1.0f, 40.0f))) {
        _newGameError.clear();
        _currentMenuScreen = MenuScreen::NewGame;
    }

    if (ImGui::Button("Load Game", ImVec2(-1.0f, 40.0f))) {
        _currentMenuScreen = MenuScreen::LoadGame;
        refreshSaveEntries();
    }

    ImGui::BeginDisabled(true);
    ImGui::Button("Settings", ImVec2(-1.0f, 40.0f));
    ImGui::EndDisabled();

    if (ImGui::Button("Quit Game", ImVec2(-1.0f, 40.0f))) {
        if (_quitCallback) {
            _quitCallback();
        }
    }

    ImGui::End();
}

void UI::drawPauseMenu() {
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    const ImVec2 windowSize(360.0f, 240.0f);

    ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f), ImGuiCond_Always,
                            ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(windowSize);

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
                                   | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("Paused", nullptr, flags)) {
        const char *title = "Paused";
        ImVec2 titleSize = ImGui::CalcTextSize(title);
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - titleSize.x) * 0.5f);
        ImGui::TextUnformatted(title);

        ImGui::Dummy(ImVec2(0.0f, 12.0f));

        if (ImGui::Button("Resume", ImVec2(-1.0f, 40.0f))) {
            if (_resumeCallback) {
                _resumeCallback();
            }
        }

        if (ImGui::Button("Save Game", ImVec2(-1.0f, 40.0f))) {
            if (_saveGameCallback) {
                _saveGameCallback();
            }
        }

        if (ImGui::Button("Back to Menu", ImVec2(-1.0f, 40.0f))) {
            _showPauseConfirm = true;
            LOG_INFO("UI", "Back to Menu clicked, showing confirmation dialog.");
        }
    }
    ImGui::End();

    if (_showPauseConfirm) {
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                                ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(420.0f, 0.0f));

        if (ImGui::Begin("Confirm Back To Menu", nullptr,
                         ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
                             | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::TextWrapped(
                "Any unsaved progress will be lost if you return to the main menu.\nDo you want to "
                "continue?");

            ImGui::Dummy(ImVec2(0.0f, 16.0f));

            if (ImGui::Button("Leave", ImVec2(140.0f, 0.0f))) {
                _backToMenuRequested = true;
                _showPauseConfirm = false;
                LOG_INFO("UI", "Back to Menu confirmed.");
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(140.0f, 0.0f))) {
                _showPauseConfirm = false;
                LOG_INFO("UI", "Back to Menu cancelled.");
            }
        }
        ImGui::End();
    }
}

void UI::drawNewGameScreen() {
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    const ImVec2 windowSize(520.0f, 360.0f);

    ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f), ImGuiCond_Always,
                            ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(windowSize);

    ImGui::Begin("New Game", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
                     | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

    ImGui::TextUnformatted("Create New World");
    ImGui::Separator();

    ImGui::InputText("World Name", _newGameNameBuffer.data(), _newGameNameBuffer.size());

    const char *worldTypeLabels[] = {"Procedural", "Real"};
    if (_worldTypeIndex != 0) {
        _worldTypeIndex = 0;
    }
    if (ImGui::BeginCombo("World Type", worldTypeLabels[_worldTypeIndex])) {
        const bool isProcedural = (_worldTypeIndex == 0);
        if (ImGui::Selectable(worldTypeLabels[0], isProcedural)) {
            _worldTypeIndex = 0;
        }
        ImGui::BeginDisabled();
        ImGui::Selectable(worldTypeLabels[1], false);
        ImGui::EndDisabled();
        ImGui::EndCombo();
    }

    const char *gameModeLabels[] = {"Career", "Sandbox"};
    if (_gameModeIndex != 1) {
        _gameModeIndex = 1;
    }
    if (ImGui::BeginCombo("Game Mode", gameModeLabels[_gameModeIndex])) {
        ImGui::BeginDisabled();
        ImGui::Selectable(gameModeLabels[0], false);
        ImGui::EndDisabled();
        const bool isSandbox = (_gameModeIndex == 1);
        if (ImGui::Selectable(gameModeLabels[1], isSandbox)) {
            _gameModeIndex = 1;
        }
        ImGui::EndCombo();
    }

    if (!_newGameError.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.87f, 0.23f, 0.23f, 1.0f));
        ImGui::TextWrapped("%s", _newGameError.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    if (ImGui::Button("Create World", ImVec2(160.0f, 40.0f))) {
        std::string worldName = _newGameNameBuffer.data();
        const bool hasNonWhitespace =
            std::any_of(worldName.begin(), worldName.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            });

        if (!hasNonWhitespace) {
            _newGameError = "Please enter a world name.";
        } else if (_startNewGameCallback) {
            NewGameOptions options;
            options.worldName = worldName;
            options.worldType = _worldTypeIndex == 0 ? WorldType::PROCEDURAL : WorldType::REAL;
            options.gameMode = _gameModeIndex == 0 ? GameMode::CAREER : GameMode::SANDBOX;
            _startNewGameCallback(options);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Back", ImVec2(160.0f, 40.0f))) {
        _currentMenuScreen = MenuScreen::Main;
    }

    ImGui::End();
}

void UI::drawLoadGameScreen() {
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    const ImVec2 windowSize(520.0f, 400.0f);

    ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f), ImGuiCond_Always,
                            ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(windowSize);

    ImGui::Begin("Load Game", nullptr,
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
                     | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

    ImGui::TextUnformatted("Select a saved game to load");
    ImGui::Separator();

    ImVec2 listSize(ImGui::GetContentRegionAvail().x, 220.0f);
    if (ImGui::BeginChild("##SaveList", listSize, true,
                          ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoSavedSettings)) {
        if (_saveEntries.empty()) {
            ImGui::TextDisabled("No saved games found in %s", _saveDirectory.string().c_str());
        } else if (ImGui::BeginTable("LoadGameTable", 2,
                                     ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH
                                         | ImGuiTableFlags_SizingStretchProp
                                         | ImGuiTableFlags_NoSavedSettings)) {
            ImGui::TableSetupColumn("Save", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthFixed, 90.0f);

            for (int i = 0; i < static_cast<int>(_saveEntries.size()); ++i) {
                ImGui::TableNextRow();

                ImGui::TableSetColumnIndex(0);
                ImGui::PushID(i);
                const bool selected = (_selectedSaveIndex == i);
                if (ImGui::Selectable(_saveEntries[i].displayName.c_str(), selected,
                                      ImGuiSelectableFlags_SpanAllColumns
                                          | ImGuiSelectableFlags_AllowDoubleClick)) {
                    _selectedSaveIndex = i;
                    _loadGameError.clear();
                }
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)
                    && _loadGameCallback) {
                    _loadGameCallback(_saveEntries[static_cast<size_t>(i)].fullPath);
                }

                ImGui::TableSetColumnIndex(1);
                bool deleted = false;
                if (ImGui::Button("Delete")) {
                    std::error_code removeError;
                    if (!std::filesystem::remove(_saveEntries[static_cast<size_t>(i)].fullPath,
                                                 removeError)) {
                        _loadGameError = "Unable to delete save file.";
                    } else {
                        if (_selectedSaveIndex == i) {
                            _selectedSaveIndex = -1;
                        }
                        refreshSaveEntries();
                        deleted = true;
                    }
                }
                ImGui::PopID();
                if (deleted) {
                    break;
                }
            }

            ImGui::EndTable();
        }
    }
    ImGui::EndChild();

    if (!_loadGameError.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.87f, 0.23f, 0.23f, 1.0f));
        ImGui::TextWrapped("%s", _loadGameError.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::Dummy(ImVec2(0.0f, 10.0f));

    if (ImGui::Button("Load Selected", ImVec2(160.0f, 40.0f))) {
        if (_selectedSaveIndex < 0
            || _selectedSaveIndex >= static_cast<int>(_saveEntries.size())) {
            _loadGameError = "Please choose a saved game to load.";
        } else if (_loadGameCallback) {
            _loadGameCallback(_saveEntries[static_cast<size_t>(_selectedSaveIndex)].fullPath);
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Back", ImVec2(160.0f, 40.0f))) {
        _currentMenuScreen = MenuScreen::Main;
        _loadGameError.clear();
    }

    ImGui::End();
}

void UI::refreshSaveEntries() {
    _saveEntries.clear();
    _selectedSaveIndex = -1;
    _loadGameError.clear();

    std::error_code ec;
    if (!std::filesystem::exists(_saveDirectory, ec)) {
        return;
    }

    for (const auto &entry : std::filesystem::directory_iterator(_saveDirectory, ec)) {
        if (ec) break;
        std::error_code entryError;
        if (!entry.is_regular_file(entryError)) continue;
        const auto path = entry.path();
        if (!path.extension().empty() && path.extension() != ".json") continue;
        _saveEntries.push_back({path.filename().string(), path});
    }

    std::sort(_saveEntries.begin(), _saveEntries.end(),
              [](const SaveEntry &a, const SaveEntry &b) {
                  return a.displayName < b.displayName;
              });
}
