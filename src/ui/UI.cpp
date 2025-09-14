#include "UI.h"
#include "Constants.h"
#include "Logger.h"
#include "app/InteractionMode.h"
#include "event/InputEvents.h"
#include "imgui-SFML.h"
#include "imgui.h"
#include "systems/rendering/TerrainRenderSystem.h"
#include "systems/world/ChunkManagerSystem.h"
#include <cstdlib>

UI::UI(sf::RenderWindow &window, TerrainRenderSystem &terrainRenderSystem,
       ServiceLocator &serviceLocator)
    : _window(window), _terrainRenderSystem(terrainRenderSystem), _serviceLocator(serviceLocator),
      _autoRegenerate(false) {
    LOG_DEBUG("UI", "UI instance created.");
    _terrainRenderSystem.setLodEnabled(_isLodEnabled);
}

void UI::drawPerformancePanel() {
    ImGui::Begin("Performance");

    auto &monitor = _serviceLocator.performanceMonitor;

    const auto &renderHistory = monitor.getHistory("Application::render");
    if (!renderHistory.empty()) {
        ImGui::PlotLines("Render (us)", renderHistory.data(), renderHistory.size(), 0, nullptr,
                         0.0f, 33000.0f, ImVec2(0, 80));
    }

    const auto &updateHistory = monitor.getHistory("Application::update");
    if (!updateHistory.empty()) {
        ImGui::PlotLines("Update (us)", updateHistory.data(), updateHistory.size(), 0, nullptr,
                         0.0f, 16000.0f, ImVec2(0, 80));
    }

    ImGui::End();
}

void UI::drawLoadingScreen() {
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    const char* message = _serviceLocator.loadingState.message.load();
    float progress = _serviceLocator.loadingState.progress.load();

    ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f),
                          ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(displaySize.x * 0.4f, 0));

    ImGui::Begin("Loading", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_AlwaysAutoResize);

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

void UI::update(sf::Time deltaTime, size_t numberOfStationsInActiveLine) {
    ImGui::SFML::Update(_window, deltaTime);

    const auto appState = _serviceLocator.gameState.currentAppState;
    if (appState == AppState::LOADING) {
        drawLoadingScreen();
        return;
    }

    drawPerformancePanel();

    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    ImGuiWindowFlags size_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                                  | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

    ImVec2 debugWindowPos = ImVec2(windowPadding, windowPadding);
    ImGui::SetNextWindowPos(debugWindowPos, ImGuiCond_Always);
    ImGui::Begin("Profiling", nullptr, size_flags);
    ImGui::Text("FPS: %.1f", 1.f / deltaTime.asSeconds());
    ImGui::Text("Zoom: %.2f", _serviceLocator.camera.getZoom());
    ImGui::End();

    float worldGenSettingsWidth = Constants::UI_WORLD_GEN_SETTINGS_WIDTH;
    ImVec2 worldGenSettingsPos =
        ImVec2(displaySize.x - worldGenSettingsWidth - windowPadding, windowPadding);
    ImGui::SetNextWindowPos(worldGenSettingsPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(worldGenSettingsWidth, 0.0f), ImGuiCond_Always);
    ImGui::Begin("World Generation Settings", nullptr, window_flags);

    auto &worldState = _serviceLocator.registry.get<WorldStateComponent>(
        _serviceLocator.registry.view<WorldStateComponent>().front());
    WorldGenParams &params = _serviceLocator.worldGenerationSystem.getParams();

    bool paramsChanged = false;

    if (ImGui::Button("New Seed")) {
        for (auto &layer : params.noiseLayers) {
            layer.seed = std::rand();
        }
        paramsChanged = true;
    }

    ImGui::Separator();

    for (int i = 0; i < params.noiseLayers.size(); ++i) {
        auto &layer = params.noiseLayers[i];
        std::string layerHeader = layer.name;
        if (ImGui::CollapsingHeader(layerHeader.c_str())) {
            ImGui::PushID(i);

            if (ImGui::SliderFloat("Frequency", &layer.frequency, 0.001f, 0.1f, "%.4f"))
                paramsChanged = true;

            const char *noiseTypes[] = {"OpenSimplex2", "OpenSimplex2S", "Cellular",
                                        "Perlin",       "ValueCubic",    "Value"};
            if (ImGui::Combo("Noise Type", reinterpret_cast<int *>(&layer.noiseType), noiseTypes,
                             IM_ARRAYSIZE(noiseTypes)))
                paramsChanged = true;

            const char *fractalTypes[] = {"None",
                                          "FBm",
                                          "Ridged",
                                          "PingPong",
                                          "DomainWarpProgressive",
                                          "DomainWarpIndependent"};
            if (ImGui::Combo("Fractal Type", reinterpret_cast<int *>(&layer.fractalType),
                             fractalTypes, IM_ARRAYSIZE(fractalTypes)))
                paramsChanged = true;

            if (ImGui::SliderInt("Octaves", &layer.octaves, 1, 10)) paramsChanged = true;
            if (ImGui::SliderFloat("Lacunarity", &layer.lacunarity, 0.1f, 4.0f))
                paramsChanged = true;
            if (ImGui::SliderFloat("Gain", &layer.gain, 0.1f, 1.0f)) paramsChanged = true;
            if (ImGui::SliderFloat("Weight", &layer.weight, 0.0f, 2.0f)) paramsChanged = true;

            ImGui::PopID();
        }
    }

    ImGui::Separator();

    if (ImGui::SliderFloat("Land Threshold", &params.landThreshold, -1.0f, 1.0f, "%.2f"))
        paramsChanged = true;
    if (ImGui::Checkbox("Distort Coastline", &params.distortCoastline)) paramsChanged = true;
    if (params.distortCoastline) {
        if (ImGui::SliderFloat("Distortion Strength", &params.coastlineDistortionStrength, 0.0f,
                               0.5f, "%.2f"))
            paramsChanged = true;
    }

    ImGui::Separator();

    bool gridChanged = false;
    if (ImGui::InputInt("World Chunks X", &params.worldDimensionsInChunks.x)) gridChanged = true;
    if (ImGui::InputInt("World Chunks Y", &params.worldDimensionsInChunks.y)) gridChanged = true;
    if (ImGui::InputInt("Chunk Size X", &params.chunkDimensionsInCells.x)) gridChanged = true;
    if (ImGui::InputInt("Chunk Size Y", &params.chunkDimensionsInCells.y)) gridChanged = true;
    if (ImGui::InputFloat("Cell Size", &params.cellSize, 1.0f, 0.0f, "%.2f")) gridChanged = true;

    if ((paramsChanged || gridChanged) && _autoRegenerate) {
        LOG_DEBUG("UI", "Settings changed, auto-regenerating world.");
        auto paramsCopy = std::make_shared<WorldGenParams>(params);
        _serviceLocator.eventBus.enqueue<RegenerateWorldRequestEvent>({paramsCopy});
    }

    ImGui::Separator();

    if (ImGui::Button("Regenerate World")) {
        LOG_DEBUG("UI", "Regenerate World button clicked.");
        auto paramsCopy = std::make_shared<WorldGenParams>(params);
        _serviceLocator.eventBus.enqueue<RegenerateWorldRequestEvent>({paramsCopy});
    }

    if (ImGui::Checkbox("Visualize Chunk Borders", &_visualizeChunkBorders)) {
        _terrainRenderSystem.setVisualizeChunkBorders(_visualizeChunkBorders);
    }
    if (ImGui::Checkbox("Visualize Cell Borders", &_visualizeCellBorders)) {
        _terrainRenderSystem.setVisualizeCellBorders(_visualizeCellBorders);
    }

    if (ImGui::Checkbox("Visualize Suitability Map", &_visualizeSuitabilityMap)) {
        _terrainRenderSystem.setVisualizeSuitabilityMap(_visualizeSuitabilityMap);
        if (_visualizeSuitabilityMap) {
            _terrainRenderSystem.setSuitabilityMapType(static_cast<TerrainRenderSystem::SuitabilityMapType>(_selectedSuitabilityMap + 1));
        } else {
            _terrainRenderSystem.setSuitabilityMapType(TerrainRenderSystem::SuitabilityMapType::None);
        }
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!_visualizeSuitabilityMap);
    const char* items[] = { "Water", "Expandability", "City Proximity", "Final", "Test" };
    if (ImGui::Combo("##SuitabilityMap", &_selectedSuitabilityMap, items, IM_ARRAYSIZE(items))) {
        _terrainRenderSystem.setSuitabilityMapType(static_cast<TerrainRenderSystem::SuitabilityMapType>(_selectedSuitabilityMap + 1));
    }
    ImGui::EndDisabled();

    if (ImGui::Checkbox("Enable LOD", &_isLodEnabled)) {
        _terrainRenderSystem.setLodEnabled(_isLodEnabled);
    }

    ImGui::Checkbox("Auto Regenerate", &_autoRegenerate);

    ImGui::End();

    float interactionModesWidth = Constants::UI_INTERACTION_MODES_WIDTH;
    float interactionModesHeight = Constants::UI_INTERACTION_MODES_HEIGHT;
    ImVec2 interactionModesPos = ImVec2((displaySize.x - interactionModesWidth) * 0.5f,
                                        displaySize.y - interactionModesHeight - windowPadding);
    ImGui::SetNextWindowPos(interactionModesPos, ImGuiCond_Always);
    ImGui::Begin("Interaction Modes", nullptr, size_flags);
    int currentMode = static_cast<int>(_serviceLocator.gameState.currentInteractionMode);

    if (ImGui::RadioButton("None", &currentMode, static_cast<int>(InteractionMode::SELECT))) {
        _serviceLocator.eventBus.enqueue(InteractionModeChangeEvent{InteractionMode::SELECT});
        LOG_DEBUG("UI", "Interaction mode change requested: None");
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Station Placement", &currentMode,
                           static_cast<int>(InteractionMode::CREATE_STATION))) {
        _serviceLocator.eventBus.enqueue(
            InteractionModeChangeEvent{InteractionMode::CREATE_STATION});
        LOG_DEBUG("UI", "Interaction mode change requested: StationPlacement");
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Line Creation", &currentMode,
                           static_cast<int>(InteractionMode::CREATE_LINE))) {
        _serviceLocator.eventBus.enqueue(InteractionModeChangeEvent{InteractionMode::CREATE_LINE});
        LOG_DEBUG("UI", "Interaction mode change requested: LineCreation");
    }
    ImGui::End();
}

void UI::renderFrame() {
    ImGui::SFML::Render(_window);
}

void UI::cleanupResources() {
    LOG_INFO("UI", "Shutting down ImGui.");
    ImGui::SFML::Shutdown();
    LOG_INFO("UI", "ImGui shutdown complete.");
}