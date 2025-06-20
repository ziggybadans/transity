#include "UI.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include "../Logger.h"
#include "../input/InteractionMode.h"
#include <cstdlib>
#include "../core/Constants.h"
#include "../event/InputEvents.h"

UI::UI(sf::RenderWindow& window, WorldGenerationSystem* worldGenSystem, GameState& gameState, EventBus& eventBus)
    : _window(window), _gameState(gameState), _eventBus(eventBus), _worldGenerationSystem(worldGenSystem) {
    LOG_INFO("UI", "UI instance created.");
    syncWithWorldState();
}

UI::~UI() {
    LOG_INFO("UI", "UI instance destroyed.");
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

void UI::processEvent(const sf::Event& sfEvent) {
    ImGui::SFML::ProcessEvent(_window, sfEvent);
}

void UI::update(sf::Time deltaTime, size_t numberOfStationsInActiveLine) {
    ImGui::SFML::Update(_window, deltaTime);

    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    ImGuiWindowFlags size_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

    ImVec2 debugWindowPos = ImVec2(windowPadding, windowPadding);
    ImGui::SetNextWindowPos(debugWindowPos, ImGuiCond_Always);
    ImGui::Begin("Profiling", nullptr, size_flags);
    ImGui::Text("FPS: %.1f", 1.f / deltaTime.asSeconds());
    ImGui::End();

    float worldGenSettingsWidth = Constants::UI_WORLD_GEN_SETTINGS_WIDTH;
    ImVec2 worldGenSettingsPos = ImVec2(displaySize.x - worldGenSettingsWidth - windowPadding, windowPadding);
    ImGui::SetNextWindowPos(worldGenSettingsPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(worldGenSettingsWidth, 0.0f), ImGuiCond_Always);
    ImGui::Begin("World Generation Settings", nullptr, window_flags);
        bool paramsChanged = false;

        if (ImGui::InputInt("Seed", &_worldGenParams.seed)) paramsChanged = true;
        if (ImGui::InputFloat("Frequency", &_worldGenParams.frequency, 0.001f, 0.1f, "%.4f")) paramsChanged = true;
        
        const char* noiseTypes[] = { "OpenSimplex2", "OpenSimplex2S", "Cellular", "Perlin", "ValueCubic", "Value" };
        if (ImGui::Combo("Noise Type", reinterpret_cast<int*>(&_worldGenParams.noiseType), noiseTypes, IM_ARRAYSIZE(noiseTypes))) paramsChanged = true;

        const char* fractalTypes[] = { "None", "FBm", "Ridged", "PingPong", "DomainWarpProgressive", "DomainWarpIndependent" };
        if (ImGui::Combo("Fractal Type", reinterpret_cast<int*>(&_worldGenParams.fractalType), fractalTypes, IM_ARRAYSIZE(fractalTypes))) paramsChanged = true;

        if (ImGui::SliderInt("Octaves", &_worldGenParams.octaves, 1, 10)) paramsChanged = true;
        if (ImGui::SliderFloat("Lacunarity", &_worldGenParams.lacunarity, 0.1f, 4.0f)) paramsChanged = true;
        if (ImGui::SliderFloat("Gain", &_worldGenParams.gain, 0.1f, 1.0f)) paramsChanged = true;
        if (ImGui::SliderFloat("Land Threshold", &_worldGenParams.landThreshold, -1.0f, 1.0f, "%.2f")) paramsChanged = true;
        if (ImGui::Checkbox("Distort Coastline", &_worldGenParams.distortCoastline)) paramsChanged = true;

        ImGui::Separator();

        bool gridChanged = false;
        if (ImGui::InputInt("World Chunks X", &_worldChunksX)) gridChanged = true;
        if (ImGui::InputInt("World Chunks Y", &_worldChunksY)) gridChanged = true;
        if (ImGui::InputInt("Chunk Size X", &_chunkSizeX)) gridChanged = true;
        if (ImGui::InputInt("Chunk Size Y", &_chunkSizeY)) gridChanged = true;
        if (ImGui::InputFloat("Cell Size", &_cellSize, 1.0f, 0.0f, "%.2f")) gridChanged = true;

        if (paramsChanged || gridChanged) {
            _eventBus.trigger(WorldGenParamsChangeEvent{
                _worldGenParams,
                _worldChunksX,
                _worldChunksY,
                _chunkSizeX,
                _chunkSizeY,
                _cellSize
            });

            if (_autoRegenerate) {
                LOG_INFO("UI", "Settings changed, auto-regenerating world.");
                _eventBus.trigger<RegenerateWorldRequestEvent>();
            }
        }

        ImGui::Separator();

        if (ImGui::Button("Regenerate World")) {
            LOG_INFO("UI", "Regenerate World button clicked.");
            _eventBus.trigger<RegenerateWorldRequestEvent>();
        }

        ImGui::Checkbox("Visualize Noise", &_visualizeNoise);
        ImGui::Checkbox("Auto Regenerate", &_autoRegenerate);

    ImGui::End();

    float interactionModesWidth = Constants::UI_INTERACTION_MODES_WIDTH;
    float interactionModesHeight = Constants::UI_INTERACTION_MODES_HEIGHT;
    ImVec2 interactionModesPos = ImVec2((displaySize.x - interactionModesWidth) * 0.5f,
        displaySize.y - interactionModesHeight - windowPadding);
    ImGui::SetNextWindowPos(interactionModesPos, ImGuiCond_Always);
    ImGui::Begin("Interaction Modes", nullptr, size_flags);
        int currentMode = static_cast<int>(_gameState.currentInteractionMode);

        if (ImGui::RadioButton("None", &currentMode, static_cast<int>(InteractionMode::SELECT))) {
            _eventBus.trigger(InteractionModeChangeEvent{InteractionMode::SELECT});
            LOG_INFO("UI", "Interaction mode change requested: None");
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Station Placement", &currentMode, static_cast<int>(InteractionMode::CREATE_STATION))) {
            _eventBus.trigger(InteractionModeChangeEvent{InteractionMode::CREATE_STATION});
            LOG_INFO("UI", "Interaction mode change requested: StationPlacement");
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Line Creation", &currentMode, static_cast<int>(InteractionMode::CREATE_LINE))) {
            _eventBus.trigger(InteractionModeChangeEvent{InteractionMode::CREATE_LINE});
            LOG_INFO("UI", "Interaction mode change requested: LineCreation");
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

void UI::syncWithWorldState() {
    if (!_worldGenerationSystem) {
        LOG_WARN("UI", "WorldGenerationSystem is null, cannot sync state.");
        return;
    }

    // Sync noise and generation settings from the single source of truth
    _worldGenParams = _worldGenerationSystem->getParams();

    // Sync grid settings
    auto& registry = _worldGenerationSystem->getRegistry();
    auto view = registry.view<WorldGridComponent>();
    if (!view.empty()) {
        auto& worldGrid = view.get<WorldGridComponent>(view.front());
        _worldChunksX = worldGrid.worldDimensionsInChunks.x;
        _worldChunksY = worldGrid.worldDimensionsInChunks.y;
        _chunkSizeX = worldGrid.chunkDimensionsInCells.x;
        _chunkSizeY = worldGrid.chunkDimensionsInCells.y;
        _cellSize = worldGrid.cellSize;
    } else {
         LOG_WARN("UI", "WorldGridComponent not found, cannot sync grid state.");
    }
}
