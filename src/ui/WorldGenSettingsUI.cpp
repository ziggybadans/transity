#include "WorldGenSettingsUI.h"
#include "Constants.h"
#include "Logger.h"
#include "event/DeletionEvents.h"
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
    bool gridChanged = false;

    drawNoiseLayerSettings(params, paramsChanged);
    ImGui::Separator();
    drawWorldGridSettings(params, gridChanged);
    ImGui::Separator();
    drawVisualizationSettings();
    ImGui::Separator();
    drawActions(params);

    if ((paramsChanged || gridChanged) && _autoRegenerate) {
        LOG_DEBUG("UI", "Settings changed, auto-regenerating world.");
        auto paramsCopy = std::make_shared<WorldGenParams>(params);
        _eventBus.enqueue<RegenerateWorldRequestEvent>({paramsCopy});
    }

    ImGui::End();
}

void WorldGenSettingsUI::drawNoiseLayerSettings(WorldGenParams &params, bool &paramsChanged) {
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
}

void WorldGenSettingsUI::drawWorldGridSettings(WorldGenParams &params, bool &gridChanged) {
    if (ImGui::SliderFloat("Land Threshold", &params.landThreshold, -1.0f, 1.0f, "%.2f"))
        gridChanged = true;
    if (ImGui::Checkbox("Distort Coastline", &params.distortCoastline)) gridChanged = true;
    if (params.distortCoastline) {
        if (ImGui::SliderFloat("Distortion Strength", &params.coastlineDistortionStrength, 0.0f,
                               0.5f, "%.2f"))
            gridChanged = true;
    }
    ImGui::Separator();
    if (ImGui::InputInt("World Chunks X", &params.worldDimensionsInChunks.x)) gridChanged = true;
    if (ImGui::InputInt("World Chunks Y", &params.worldDimensionsInChunks.y)) gridChanged = true;
    if (ImGui::InputInt("Chunk Size X", &params.chunkDimensionsInCells.x)) gridChanged = true;
    if (ImGui::InputInt("Chunk Size Y", &params.chunkDimensionsInCells.y)) gridChanged = true;
    if (ImGui::InputFloat("Cell Size", &params.cellSize, 1.0f, 0.0f, "%.2f")) gridChanged = true;
}

void WorldGenSettingsUI::drawVisualizationSettings() {
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
    const char *items[] = {"Water", "Expandability", "City Proximity", "Noise",
                           "Final", "Town",          "Suburb"};
    if (ImGui::Combo("##SuitabilityMap", &_selectedSuitabilityMap, items, IM_ARRAYSIZE(items))) {
        _terrainRenderSystem.setSuitabilityMapType(
            static_cast<TerrainRenderSystem::SuitabilityMapType>(_selectedSuitabilityMap + 1));
    }
    ImGui::EndDisabled();

    if (ImGui::Checkbox("Enable LOD", &_isLodEnabled)) {
        _terrainRenderSystem.setLodEnabled(_isLodEnabled);
    }
}

void WorldGenSettingsUI::drawActions(const WorldGenParams &params) {
    if (ImGui::Button("Regenerate World")) {
        LOG_DEBUG("UI", "Regenerate World button clicked.");
        auto paramsCopy = std::make_shared<WorldGenParams>(params);
        _eventBus.enqueue<RegenerateWorldRequestEvent>({paramsCopy});
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto Regenerate", &_autoRegenerate);
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) ImColor::HSV(0.0f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4) ImColor::HSV(0.0f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4) ImColor::HSV(0.0f, 0.8f, 0.8f));
    if (ImGui::Button("Delete All Entities")) {
        ImGui::OpenPopup("Delete All Confirmation");
    }
    ImGui::PopStyleColor(3);

    if (ImGui::BeginPopupModal("Delete All Confirmation", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text(
            "Are you sure you want to delete all entities?\nThis action cannot be undone.\n\n");
        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4) ImColor::HSV(0.0f, 0.6f, 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4) ImColor::HSV(0.0f, 0.7f, 0.7f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4) ImColor::HSV(0.0f, 0.8f, 0.8f));
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            _eventBus.enqueue<DeleteAllEntitiesEvent>({});
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(3);
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}