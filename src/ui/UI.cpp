#include "UI.h"
#include "Logger.h"
#include "app/LoadingState.h"
#include "imgui-SFML.h"
#include "imgui.h"

UI::UI(sf::RenderWindow &window, LoadingState &loadingState)
    : _window(window), _loadingState(loadingState) {
    LOG_DEBUG("UI", "UI instance created.");
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

    if (appState == AppState::LOADING) {
        drawLoadingScreen();
    }
}

void UI::renderFrame() {
    ImGui::SFML::Render(_window);
}

void UI::cleanupResources() {
    LOG_INFO("UI", "Shutting down ImGui.");
    ImGui::SFML::Shutdown();
    LOG_INFO("UI", "ImGui shutdown complete.");
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
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove
                     | ImGuiWindowFlags_AlwaysAutoResize);

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