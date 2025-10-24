#include "WorldGenSettingsUI.h"
#include "Constants.h"
#include "Logger.h"
#include "event/DeletionEvents.h"
#include "event/InputEvents.h"
#include "event/UIEvents.h"
#include "imgui.h"
#include "systems/rendering/TerrainRenderSystem.h"
#include "systems/world/WorldGenerationSystem.h"
#include <cstdlib>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <string>
#include <vector>

WorldGenSettingsUI::WorldGenSettingsUI(EventBus &eventBus,
                                       WorldGenerationSystem &worldGenerationSystem,
                                       TerrainRenderSystem &terrainRenderSystem)
    : _eventBus(eventBus), _worldGenerationSystem(worldGenerationSystem),
      _terrainRenderSystem(terrainRenderSystem),
      _defaultParams(worldGenerationSystem.getParams()) {
    _shadedReliefEnabled = _terrainRenderSystem.isShadedReliefEnabled();
    std::fill(_fileDialogNameBuffer.begin(), _fileDialogNameBuffer.end(), '\0');
    const std::string defaultName = "savegame.json";
    std::copy(defaultName.begin(), defaultName.end(), _fileDialogNameBuffer.begin());
    _fileDialogDirectory = getDefaultSaveDirectory();
    _mouseMovedConnection =
        _eventBus.sink<MouseMovedEvent>().connect<&WorldGenSettingsUI::onMouseMoved>(this);
    LOG_DEBUG("WorldGenSettingsUI", "WorldGenSettingsUI instance created.");
}

WorldGenSettingsUI::~WorldGenSettingsUI() {
    _mouseMovedConnection.release();
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
    drawElevationSettings(params, paramsChanged);
    ImGui::Separator();
    drawVisualizationSettings();
    ImGui::Separator();
    drawActions(params);

    if ((paramsChanged || gridChanged) && _autoRegenerate) {
        LOG_DEBUG("UI", "Settings changed, auto-regenerating world.");
        auto paramsCopy = std::make_shared<WorldGenParams>(params);
        _eventBus.enqueue<RegenerateWorldRequestEvent>({paramsCopy});
    }
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    _lastWindowBottomY = windowPos.y + windowSize.y;

    drawFileDialog();
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

void WorldGenSettingsUI::drawElevationSettings(WorldGenParams &params, bool &paramsChanged) {
    ImGui::TextUnformatted("Elevation");
    ImGui::Spacing();

    if (sliderFloatWithReset("Max Elevation", &params.elevation.maxElevation,
                             _defaultParams.elevation.maxElevation, 0.0f, 2000.0f, "%.0f")) {
        paramsChanged = true;
    }
    if (sliderFloatWithReset("Elevation Exponent", &params.elevation.elevationExponent,
                             _defaultParams.elevation.elevationExponent, 0.1f, 5.0f, "%.2f")) {
        paramsChanged = true;
    }

    if (ImGui::Checkbox("Shaded Relief Map", &_shadedReliefEnabled)) {
        _terrainRenderSystem.setShadedReliefEnabled(_shadedReliefEnabled);
        _eventBus.enqueue<ImmediateRedrawEvent>({});
    }

    const float cellSize = params.cellSize;
    const int cellsX = params.worldDimensionsInChunks.x * params.chunkDimensionsInCells.x;
    const int cellsY = params.worldDimensionsInChunks.y * params.chunkDimensionsInCells.y;

    if (!_hasMouseWorldPos || cellSize <= 0.0f) {
        ImGui::TextUnformatted("Cell Elevation: (move cursor over world)");
        return;
    }

    int cellX = static_cast<int>(std::floor(_lastMouseWorldPos.x / cellSize));
    int cellY = static_cast<int>(std::floor(_lastMouseWorldPos.y / cellSize));

    if (cellX < 0 || cellY < 0 || cellX >= cellsX || cellY >= cellsY) {
        ImGui::TextUnformatted("Cell Elevation: (outside world)");
        return;
    }

    const float sampleX = (static_cast<float>(cellX) + 0.5f) * cellSize;
    const float sampleY = (static_cast<float>(cellY) + 0.5f) * cellSize;
    const float elevation = _worldGenerationSystem.getElevationAt(sampleX, sampleY);
    ImGui::Text("Cell [%d, %d] Elevation: %.1f", cellX, cellY, elevation);

    if (params.elevation.maxElevation > 0.0f) {
        float normalized = std::clamp(elevation / params.elevation.maxElevation, 0.0f, 1.0f);
        ImGui::SameLine();
        ImGui::Text("(%.0f%% of max)", normalized * 100.0f);
    }
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

void WorldGenSettingsUI::onMouseMoved(const MouseMovedEvent &event) {
    _lastMouseWorldPos = event.worldPosition;
    _hasMouseWorldPos = true;
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

    ImGui::Separator();
    ImGui::TextUnformatted("Save / Load Game");
    if (ImGui::Button("Save Game")) {
        openFileDialog(FileDialogMode::Save);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load Game")) {
        openFileDialog(FileDialogMode::Load);
    }
}

void WorldGenSettingsUI::openFileDialog(FileDialogMode mode) {
    _fileDialogMode = mode;
    _fileDialogError.clear();
    _fileDialogSelected.clear();
    _fileDialogDirectory = getDefaultSaveDirectory();
    _fileDialogScrollToTop = true;

    std::fill(_fileDialogNameBuffer.begin(), _fileDialogNameBuffer.end(), '\0');
    if (mode == FileDialogMode::Save) {
        const std::string defaultName = "savegame.json";
        std::copy(defaultName.begin(),
                  defaultName.begin() + std::min(defaultName.size(), _fileDialogNameBuffer.size() - 1),
                  _fileDialogNameBuffer.begin());
        std::error_code ec;
        std::filesystem::create_directories(_fileDialogDirectory, ec);
    }

    ImGui::OpenPopup("Game File Dialog");
}

void WorldGenSettingsUI::drawFileDialog() {
    if (_fileDialogMode == FileDialogMode::None) {
        return;
    }

    const bool isSave = (_fileDialogMode == FileDialogMode::Save);
    const ImGuiWindowFlags flags =
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::BeginPopupModal("Game File Dialog", nullptr, flags)) {
        ImGui::TextUnformatted(isSave ? "Save Game" : "Load Game");
        ImGui::Spacing();

        const std::filesystem::path parentDir = _fileDialogDirectory.parent_path();
        const bool canGoUp = !parentDir.empty() && parentDir != _fileDialogDirectory;

        if (!canGoUp) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Up")) {
            _fileDialogDirectory = parentDir;
            _fileDialogSelected.clear();
            _fileDialogError.clear();
            _fileDialogScrollToTop = true;
            if (!isSave) {
                std::fill(_fileDialogNameBuffer.begin(), _fileDialogNameBuffer.end(), '\0');
            }
        }
        if (!canGoUp) {
            ImGui::EndDisabled();
        }
        ImGui::SameLine();
        ImGui::TextWrapped("%s", _fileDialogDirectory.string().c_str());
        ImGui::Separator();

        struct DirEntry {
            std::string name;
            bool isDirectory = false;
        };
        std::vector<DirEntry> entries;
        std::error_code ec;
        if (std::filesystem::exists(_fileDialogDirectory, ec)) {
            for (const auto &entry : std::filesystem::directory_iterator(_fileDialogDirectory, ec)) {
                std::error_code entryError;
                const auto filename = entry.path().filename().string();
                if (entry.is_directory(entryError)) {
                    entries.push_back({filename, true});
                    continue;
                }
                if (!entry.is_regular_file(entryError)) continue;
                if (!entry.path().extension().empty() && entry.path().extension() != ".json") {
                    continue;
                }
                entries.push_back({filename, false});
            }
        }

        std::sort(entries.begin(), entries.end(),
                  [](const DirEntry &a, const DirEntry &b) {
                      if (a.isDirectory != b.isDirectory) {
                          return a.isDirectory > b.isDirectory;
                      }
                      return a.name < b.name;
                  });

        ImVec2 listSize(ImGui::GetContentRegionAvail().x, 200.0f);
        if (ImGui::BeginChild("##GameFileList", listSize, true,
                              ImGuiWindowFlags_HorizontalScrollbar)) {
            if (_fileDialogScrollToTop) {
                ImGui::SetScrollY(0.0f);
                _fileDialogScrollToTop = false;
            }

            if (entries.empty()) {
                ImGui::TextDisabled("No files found.");
            } else {
                for (const auto &entry : entries) {
                    bool selected = (_fileDialogSelected == entry.name);
                    std::string label =
                        entry.isDirectory ? "[Dir] " + entry.name : entry.name;
                    if (ImGui::Selectable(label.c_str(), selected,
                                          ImGuiSelectableFlags_AllowDoubleClick)) {
                        if (entry.isDirectory) {
                            _fileDialogDirectory = _fileDialogDirectory / entry.name;
                            _fileDialogSelected.clear();
                            _fileDialogError.clear();
                            _fileDialogScrollToTop = true;
                            std::fill(_fileDialogNameBuffer.begin(),
                                      _fileDialogNameBuffer.end(), '\0');
                            ImGui::EndChild();
                            ImGui::EndPopup();
                            return;
                        }

                        _fileDialogSelected = entry.name;
                        std::fill(_fileDialogNameBuffer.begin(), _fileDialogNameBuffer.end(),
                                  '\0');
                        std::copy(entry.name.begin(),
                                  entry.name.begin()
                                      + std::min(entry.name.size(),
                                                 _fileDialogNameBuffer.size() - 1),
                                  _fileDialogNameBuffer.begin());
                        if (!isSave && ImGui::IsMouseDoubleClicked(0)) {
                            ImGui::EndChild();
                            const std::filesystem::path fullPath =
                                _fileDialogDirectory / entry.name;
                            LOG_INFO("UI", "Load Game requested for path: %s",
                                     fullPath.string().c_str());
                            _eventBus.enqueue<LoadGameRequestEvent>({fullPath.string()});
                            _fileDialogMode = FileDialogMode::None;
                            _fileDialogError.clear();
                            _fileDialogSelected.clear();
                            ImGui::CloseCurrentPopup();
                            ImGui::EndPopup();
                            return;
                        }
                    }
                }
            }
        }
        ImGui::EndChild();

        if (isSave) {
            ImGui::InputText("File Name", _fileDialogNameBuffer.data(),
                             _fileDialogNameBuffer.size());
        } else {
            ImGui::InputText("Selected File", _fileDialogNameBuffer.data(),
                             _fileDialogNameBuffer.size(), ImGuiInputTextFlags_ReadOnly);
        }

        if (!_fileDialogError.empty()) {
            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.87f, 0.23f, 0.23f, 1.0f));
            ImGui::TextWrapped("%s", _fileDialogError.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::Separator();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            _fileDialogMode = FileDialogMode::None;
            _fileDialogError.clear();
            _fileDialogSelected.clear();
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            return;
        }

        ImGui::SameLine();
        const char *actionLabel = isSave ? "Save" : "Load";
        bool disableAction = (!isSave && _fileDialogSelected.empty());
        if (disableAction) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button(actionLabel, ImVec2(120, 0))) {
            std::string inputName = _fileDialogNameBuffer.data();
            if (!isSave && inputName.empty() && !_fileDialogSelected.empty()) {
                inputName = _fileDialogSelected;
            }

            if (isSave) {
                std::string sanitized = sanitizeFilename(inputName);
                if (sanitized.empty()) {
                    _fileDialogError = "Please enter a valid file name.";
                } else {
                    if (std::filesystem::path(sanitized).extension().empty()) {
                        sanitized += ".json";
                    }
                    std::error_code createError;
                    std::filesystem::create_directories(_fileDialogDirectory, createError);
                    if (createError) {
                        _fileDialogError = "Unable to create save directory.";
                    } else {
                        std::fill(_fileDialogNameBuffer.begin(), _fileDialogNameBuffer.end(),
                                  '\0');
                        std::copy(sanitized.begin(),
                                  sanitized.begin() + std::min(sanitized.size(),
                                                               _fileDialogNameBuffer.size() - 1),
                                  _fileDialogNameBuffer.begin());
                        const std::filesystem::path fullPath = _fileDialogDirectory / sanitized;
                        LOG_INFO("UI", "Save Game requested for path: %s",
                                 fullPath.string().c_str());
                        _eventBus.enqueue<SaveGameRequestEvent>({fullPath.string()});
                        _fileDialogMode = FileDialogMode::None;
                        _fileDialogError.clear();
                        _fileDialogSelected.clear();
                        ImGui::CloseCurrentPopup();
                        ImGui::EndPopup();
                        return;
                    }
                }
            } else {
                std::filesystem::path fullPath = _fileDialogDirectory / inputName;
                std::error_code existsError;
                if (inputName.empty()) {
                    _fileDialogError = "Please select a file to load.";
                } else if (!std::filesystem::exists(fullPath, existsError)
                           || !std::filesystem::is_regular_file(fullPath, existsError)) {
                    _fileDialogError = "Selected file does not exist.";
                } else {
                    LOG_INFO("UI", "Load Game requested for path: %s", fullPath.string().c_str());
                    _eventBus.enqueue<LoadGameRequestEvent>({fullPath.string()});
                    _fileDialogMode = FileDialogMode::None;
                    _fileDialogError.clear();
                    _fileDialogSelected.clear();
                    ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                    return;
                }
            }
        }
        if (disableAction) {
            ImGui::EndDisabled();
        }

        ImGui::EndPopup();
    } else {
        _fileDialogMode = FileDialogMode::None;
    }
}

std::filesystem::path WorldGenSettingsUI::getDefaultSaveDirectory() const {
    return std::filesystem::current_path() / "saves";
}

std::string WorldGenSettingsUI::sanitizeFilename(const std::string &name) const {
    std::string result;
    result.reserve(name.size());
    for (char ch : name) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (std::isalnum(c) || ch == '_' || ch == '-' || ch == '.') {
            result.push_back(ch);
        } else if (std::isspace(c)) {
            result.push_back('_');
        }
    }

    while (!result.empty()
           && (result.back() == '.' || result.back() == '_' || result.back() == ' ')) {
        result.pop_back();
    }

    return result;
}
