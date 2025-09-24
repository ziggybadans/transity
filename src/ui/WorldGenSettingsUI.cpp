#include "WorldGenSettingsUI.h"
#include "Constants.h"
#include "Logger.h"
#include "event/UIEvents.h"
#include "imgui.h"
#include "systems/rendering/TerrainRenderSystem.h"
#include "systems/world/WorldGenerationSystem.h"
#include <cstdlib>

WorldGenSettingsUI::WorldGenSettingsUI(EventBus &eventBus,
                                       WorldGenerationSystem &worldGenerationSystem,
                                       TerrainRenderSystem &terrainRenderSystem)
    : _eventBus(eventBus), _worldGenerationSystem(worldGenerationSystem),
      _terrainRenderSystem(terrainRenderSystem) {
    LOG_DEBUG("WorldGenSettingsUI", "WorldGenSettingsUI instance created.");
    _terrainRenderSystem.setLodEnabled(_isLodEnabled);
}

WorldGenSettingsUI::~WorldGenSettingsUI() {
    LOG_DEBUG("WorldGenSettingsUI", "WorldGenSettingsUI instance destroyed.");
}

void WorldGenSettingsUI::draw() {
    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    float worldGenSettingsWidth = Constants::UI_WORLD_GEN_SETTINGS_WIDTH;
    ImVec2 worldGenSettingsPos =
        ImVec2(displaySize.x - worldGenSettingsWidth - windowPadding, windowPadding);
    ImGui::SetNextWindowPos(worldGenSettingsPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(worldGenSettingsWidth, 0.0f), ImGuiCond_Always);
    ImGui::Begin("World Generation Settings", nullptr, window_flags);

    WorldGenParams &params = _worldGenerationSystem.getParams();
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
        _eventBus.enqueue<RegenerateWorldRequestEvent>({paramsCopy});
    }

    ImGui::Separator();

    if (ImGui::Button("Regenerate World")) {
        LOG_DEBUG("UI", "Regenerate World button clicked.");
        auto paramsCopy = std::make_shared<WorldGenParams>(params);
        _eventBus.enqueue<RegenerateWorldRequestEvent>({paramsCopy});
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
            _terrainRenderSystem.setSuitabilityMapType(
                static_cast<TerrainRenderSystem::SuitabilityMapType>(_selectedSuitabilityMap + 1));
        } else {
            _terrainRenderSystem.setSuitabilityMapType(
                TerrainRenderSystem::SuitabilityMapType::None);
        }
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!_visualizeSuitabilityMap);
    const char *items[] = {"Water", "Expandability", "City Proximity", "Noise", "Final"};
    if (ImGui::Combo("##SuitabilityMap", &_selectedSuitabilityMap, items, IM_ARRAYSIZE(items))) {
        _terrainRenderSystem.setSuitabilityMapType(
            static_cast<TerrainRenderSystem::SuitabilityMapType>(_selectedSuitabilityMap + 1));
    }
    ImGui::EndDisabled();

    if (ImGui::Checkbox("Enable LOD", &_isLodEnabled)) {
        _terrainRenderSystem.setLodEnabled(_isLodEnabled);
    }

    ImGui::Checkbox("Auto Regenerate", &_autoRegenerate);

    ImGui::End();
}