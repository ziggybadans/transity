#include "DebugUI.h"
#include "Constants.h"
#include "Logger.h"
#include "app/GameState.h"
#include "core/PerformanceMonitor.h"
#include "imgui.h"
#include "render/Camera.h"
#include "render/ColorManager.h"

DebugUI::DebugUI(PerformanceMonitor &performanceMonitor, Camera &camera, GameState &gameState,
                 ColorManager &colorManager, EventBus &eventBus, sf::RenderWindow &window)
    : _performanceMonitor(performanceMonitor), _camera(camera), _gameState(gameState),
      _colorManager(colorManager), _window(window) {
    _themeChangedConnection =
        eventBus.sink<ThemeChangedEvent>().connect<&DebugUI::onThemeChanged>(this);
    LOG_DEBUG("DebugUI", "DebugUI instance created.");
}

DebugUI::~DebugUI() {
    LOG_DEBUG("DebugUI", "DebugUI instance destroyed.");
}

void DebugUI::draw(sf::Time deltaTime) {
    drawTimeControlWindow();
    drawProfilingWindow(deltaTime);
    drawSettingsWindow();
}

void DebugUI::onThemeChanged(const ThemeChangedEvent &event) {
    if (event.theme == Theme::Light) {
        ImGui::StyleColorsLight();
    } else {
        ImGui::StyleColorsDark();
    }
}

void DebugUI::drawProfilingWindow(sf::Time deltaTime) {
    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiWindowFlags size_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                                  | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

    ImGui::SetNextWindowPos(ImVec2(windowPadding, windowPadding));
    ImGui::Begin("Time Controls", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
                     | ImGuiWindowFlags_AlwaysAutoResize);
    ImVec2 timeControlWindowSize = ImGui::GetWindowSize();
    ImGui::End();

    ImVec2 debugWindowPos =
        ImVec2(windowPadding, windowPadding + timeControlWindowSize.y + windowPadding);
    ImGui::SetNextWindowPos(debugWindowPos, ImGuiCond_Always);
    ImGui::Begin("Profiling", nullptr, size_flags);
    ImGui::Text("FPS: %.1f", 1.f / deltaTime.asSeconds());
    ImGui::Text("Zoom: %.2f", _camera.getZoom());

    if (ImGui::CollapsingHeader("Performance Graphs")) {
        const auto &renderHistory = _performanceMonitor.getHistory("Application::render");
        if (!renderHistory.empty()) {
            ImGui::PlotLines("Render Time (us)", renderHistory.data(), renderHistory.size(), 0,
                             nullptr, 0.0f, 33000.0f, ImVec2(0, 80));
        }
        const auto &updateHistory = _performanceMonitor.getHistory("Application::update");
        if (!updateHistory.empty()) {
            ImGui::PlotLines("Update Time (us)", updateHistory.data(), updateHistory.size(), 0,
                             nullptr, 0.0f, 16000.0f, ImVec2(0, 80));
        }
    }
    ImGui::End();
}

void DebugUI::drawTimeControlWindow() {
    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::SetNextWindowPos(ImVec2(windowPadding, windowPadding));
    ImGui::Begin("Time Controls", nullptr, flags);

    float currentMultiplier = _gameState.timeMultiplier;
    ImVec4 activeColor = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

    if (currentMultiplier == 0.0f) ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
    if (ImGui::Button("||")) _gameState.timeMultiplier = 0.0f;
    if (currentMultiplier == 0.0f) ImGui::PopStyleColor();

    ImGui::SameLine();

    if (currentMultiplier == 1.0f) ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
    if (ImGui::Button("1x")) _gameState.timeMultiplier = 1.0f;
    if (currentMultiplier == 1.0f) ImGui::PopStyleColor();

    ImGui::SameLine();

    if (currentMultiplier == 2.0f) ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
    if (ImGui::Button("2x")) _gameState.timeMultiplier = 2.0f;
    if (currentMultiplier == 2.0f) ImGui::PopStyleColor();

    ImGui::SameLine();

    if (currentMultiplier == 3.0f) ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
    if (ImGui::Button("3x")) _gameState.timeMultiplier = 3.0f;
    if (currentMultiplier == 3.0f) ImGui::PopStyleColor();

    ImGui::End();
}

void DebugUI::drawSettingsWindow() {
    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                             | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

    ImVec2 settingsWindowPos =
        ImVec2(windowPadding,
               _window.getSize().y - ImGui::GetFrameHeightWithSpacing() * 2.5 - windowPadding);
    ImGui::SetNextWindowPos(settingsWindowPos, ImGuiCond_Always);
    ImGui::Begin("Settings", nullptr, flags);

    ImGui::Text("Theme");
    ImGui::SameLine();

    int currentTheme = static_cast<int>(_colorManager.getTheme());
    if (ImGui::RadioButton("Light", &currentTheme, static_cast<int>(Theme::Light))) {
        _colorManager.setTheme(Theme::Light);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Dark", &currentTheme, static_cast<int>(Theme::Dark))) {
        _colorManager.setTheme(Theme::Dark);
    }
    ImGui::End();
}