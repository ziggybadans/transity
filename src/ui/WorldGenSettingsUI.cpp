#include "WorldGenSettingsUI.h"
#include "Constants.h"
#include "Logger.h"
#include "event/DeletionEvents.h"
#include "imgui.h"
#include "systems/rendering/TerrainRenderSystem.h"
#include "systems/world/WorldGenerationSystem.h"
#include <cstdlib>

#include <algorithm>
#include <string>

WorldGenSettingsUI::WorldGenSettingsUI(EventBus &eventBus,
                                       WorldGenerationSystem &worldGenerationSystem,
                                       TerrainRenderSystem &terrainRenderSystem)
    : _eventBus(eventBus), _worldGenerationSystem(worldGenerationSystem),
      _terrainRenderSystem(terrainRenderSystem),
      _defaultParams(worldGenerationSystem.getParams()) {
    LOG_DEBUG("WorldGenSettingsUI", "WorldGenSettingsUI instance created.");
}

WorldGenSettingsUI::~WorldGenSettingsUI() {
    LOG_DEBUG("WorldGenSettingsUI", "WorldGenSettingsUI instance destroyed.");
}

bool WorldGenSettingsUI::drawResetButton(const char *label) {
    ImGui::PushID(label);
    bool clicked = ImGui::Button("R");
    ImGui::PopID();
    return clicked;
}

bool WorldGenSettingsUI::sliderFloatWithReset(const char *label, float *value, float defaultValue,
                                              float min, float max, const char *format) {
    const char *effectiveFormat = format ? format : "%.3f";
    bool resetClicked = drawResetButton(label);
    ImGui::SameLine();
    ImGuiStyle &style = ImGui::GetStyle();
    float availableWidth = ImGui::GetContentRegionAvail().x;
    float labelWidth = ImGui::CalcTextSize(label).x;
    float sliderWidth = std::max(0.0f, availableWidth - labelWidth - style.ItemInnerSpacing.x * 2.0f);
    ImGui::SetNextItemWidth(sliderWidth);
    bool changed = ImGui::SliderFloat(label, value, min, max, effectiveFormat);

    if (resetClicked && (*value != defaultValue)) {
        *value = defaultValue;
        changed = true;
    }

    return changed;
}

bool WorldGenSettingsUI::sliderIntWithReset(const char *label, int *value, int defaultValue,
                                            int min, int max, const char *format) {
    const char *effectiveFormat = format ? format : "%d";
    bool resetClicked = drawResetButton(label);
    ImGui::SameLine();
    ImGuiStyle &style = ImGui::GetStyle();
    float availableWidth = ImGui::GetContentRegionAvail().x;
    float labelWidth = ImGui::CalcTextSize(label).x;
    float sliderWidth = std::max(0.0f, availableWidth - labelWidth - style.ItemInnerSpacing.x * 2.0f);
    ImGui::SetNextItemWidth(sliderWidth);
    bool changed = ImGui::SliderInt(label, value, min, max, effectiveFormat);

    if (resetClicked && (*value != defaultValue)) {
        *value = defaultValue;
        changed = true;
    }

    return changed;
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
        if (!params.noiseLayers.empty()) {
            LOG_INFO("WorldGenSettingsUI", "Generated new noise seeds. Primary seed set to %d.",
                     params.noiseLayers.front().seed);
        } else {
            LOG_INFO("WorldGenSettingsUI", "Generated new noise seeds for empty noise layer set.");
        }
        paramsChanged = true;
    }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(140.0f);
    if (params.noiseLayers.empty()) {
        int dummySeed = 0;
        ImGui::BeginDisabled();
        ImGui::InputInt("Seed", &dummySeed);
        ImGui::EndDisabled();
    } else {
        if (ImGui::InputInt("Seed", &params.noiseLayers.front().seed)) {
            paramsChanged = true;
        }
    }
    ImGui::Separator();
    for (int i = 0; i < params.noiseLayers.size(); ++i) {
        auto &layer = params.noiseLayers[i];
        std::string layerHeader = layer.name;
        if (ImGui::CollapsingHeader(layerHeader.c_str())) {
            ImGui::PushID(i);

            const NoiseLayer *defaultLayer =
                (i < _defaultParams.noiseLayers.size()) ? &_defaultParams.noiseLayers[i] : nullptr;

            bool isErosionLayer = (layer.name == "Erosion");

            if (sliderFloatWithReset("Frequency", &layer.frequency,
                                     defaultLayer ? defaultLayer->frequency : layer.frequency,
                                     0.001f, 0.1f, "%.4f"))
                paramsChanged = true;

            if (!isErosionLayer) {
                if (sliderIntWithReset("Octaves", &layer.octaves,
                                       defaultLayer ? defaultLayer->octaves : layer.octaves, 1, 10))
                    paramsChanged = true;
                if (sliderFloatWithReset("Lacunarity", &layer.lacunarity,
                                         defaultLayer ? defaultLayer->lacunarity : layer.lacunarity,
                                         0.1f, 4.0f))
                    paramsChanged = true;
                if (sliderFloatWithReset("Gain", &layer.gain,
                                         defaultLayer ? defaultLayer->gain : layer.gain, 0.1f, 1.0f))
                    paramsChanged = true;
            }
            if (sliderFloatWithReset("Weight", &layer.weight,
                                     defaultLayer ? defaultLayer->weight : layer.weight, 0.0f,
                                     2.0f))
                paramsChanged = true;

            ImGui::PopID();
        }
    }
}

void WorldGenSettingsUI::drawWorldGridSettings(WorldGenParams &params, bool &gridChanged) {
    if (sliderFloatWithReset("Land Threshold", &params.landThreshold,
                             _defaultParams.landThreshold, -1.0f, 1.0f, "%.2f"))
        gridChanged = true;
    if (sliderFloatWithReset(
            "Coastline Distortion", &params.coastlineDistortionStrength,
            _defaultParams.coastlineDistortionStrength, 0.0f, 0.5f, "%.2f"))
        gridChanged = true;
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
    const ImGuiStyle &style = ImGui::GetStyle();
    const float desiredWidth =
        ImGui::CalcTextSize("City Proximity").x + style.FramePadding.x * 6.0f;
    ImGui::SetNextItemWidth(desiredWidth);
    const char *items[] = {"Water", "Expandability", "City Proximity", "Noise",
                           "Final", "Town",          "Suburb"};
    if (ImGui::Combo("##SuitabilityMap", &_selectedSuitabilityMap, items, IM_ARRAYSIZE(items))) {
        _terrainRenderSystem.setSuitabilityMapType(
            static_cast<TerrainRenderSystem::SuitabilityMapType>(_selectedSuitabilityMap + 1));
    }
    ImGui::EndDisabled();
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
    ImGui::SameLine();
    if (ImGui::Button("Regenerate Entities")) {
        LOG_INFO("UI", "Regenerate Entities button clicked.");
        _eventBus.enqueue<RegenerateEntitiesEvent>({});
    }

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
