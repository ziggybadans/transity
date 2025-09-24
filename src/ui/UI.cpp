#include "UI.h"
#include "Constants.h"
#include "Logger.h"
#include "app/Game.h"
#include "app/InteractionMode.h"
#include "app/LoadingState.h"
#include "components/GameLogicComponents.h"
#include "components/PassengerComponents.h"
#include "core/PerformanceMonitor.h"
#include "event/DeletionEvents.h"
#include "event/InputEvents.h"
#include "imgui-SFML.h"
#include "imgui.h"
#include "render/ColorManager.h"
#include "systems/gameplay/LineCreationSystem.h"
#include "systems/rendering/TerrainRenderSystem.h"
#include "systems/world/ChunkManagerSystem.h"
#include <cstdint>
#include <cstdlib>

// Add this helper function at the top of the file, after the includes
const char *trainStateToString(TrainState state) {
    switch (state) {
    case TrainState::STOPPED:
        return "Stopped";
    case TrainState::ACCELERATING:
        return "Accelerating";
    case TrainState::MOVING:
        return "Moving";
    case TrainState::DECELERATING:
        return "Decelerating";
    default:
        return "Unknown";
    }
}

UI::UI(sf::RenderWindow &window, TerrainRenderSystem &terrainRenderSystem, Game &game,
       EventBus &eventBus, GameState &gameState, LoadingState &loadingState, Camera &camera,
       PerformanceMonitor &performanceMonitor, ColorManager &colorManager,
       WorldGenerationSystem &worldGenerationSystem)
    : _window(window), _terrainRenderSystem(terrainRenderSystem), _game(game), _eventBus(eventBus),
      _gameState(gameState), _loadingState(loadingState), _camera(camera),
      _performanceMonitor(performanceMonitor), _colorManager(colorManager),
      _worldGenerationSystem(worldGenerationSystem), _autoRegenerate(false) {
    LOG_DEBUG("UI", "UI instance created.");
    _terrainRenderSystem.setLodEnabled(_isLodEnabled);

    _themeChangedConnection =
        _eventBus.sink<ThemeChangedEvent>().connect<&UI::onThemeChanged>(this);
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

void UI::update(sf::Time deltaTime) {
    ImGui::SFML::Update(_window, deltaTime);

    size_t numberOfStationsInActiveLine = 0;
    if (auto *lineCreationSystem = _game.getSystemManager().getSystem<LineCreationSystem>()) {
        lineCreationSystem->getActiveLineStations(
            [&numberOfStationsInActiveLine](entt::entity) { numberOfStationsInActiveLine++; });
    }

    const auto appState = _gameState.currentAppState;
    if (appState == AppState::LOADING) {
        drawLoadingScreen();
        return;
    }

    drawTimeControlWindow();
    drawProfilingWindow(deltaTime);
    drawWorldGenSettingsWindow();
    drawInteractionModeWindow();
    drawLineCreationWindow(numberOfStationsInActiveLine);
    drawPassengerCreationWindow();  // Add this line
    drawSettingsWindow();
    drawInfoPanel();
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

void UI::drawProfilingWindow(sf::Time deltaTime) {
    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiWindowFlags size_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                                  | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

    // Get the size of the time control window to position the profiling window below it
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

void UI::drawWorldGenSettingsWindow() {
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

void UI::drawInteractionModeWindow() {
    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    ImGuiWindowFlags size_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                                  | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

    float interactionModesWidth = Constants::UI_INTERACTION_MODES_WIDTH;
    float interactionModesHeight = Constants::UI_INTERACTION_MODES_HEIGHT;
    ImVec2 interactionModesPos =
        ImVec2((displaySize.x - interactionModesWidth) * 0.5f,
               _window.getSize().y - ImGui::GetFrameHeightWithSpacing() * 2.5 - windowPadding);
    ImGui::SetNextWindowPos(interactionModesPos, ImGuiCond_Always);
    ImGui::Begin("Interaction Modes", nullptr, size_flags);
    int currentMode = static_cast<int>(_gameState.currentInteractionMode);

    if (ImGui::RadioButton("None", &currentMode, static_cast<int>(InteractionMode::SELECT))) {
        _eventBus.enqueue(InteractionModeChangeEvent{InteractionMode::SELECT});
        LOG_DEBUG("UI", "Interaction mode change requested: None");
    }
    /*
    ImGui::SameLine();
    if (ImGui::RadioButton("Station Placement", &currentMode,
                           static_cast<int>(InteractionMode::CREATE_STATION))) {
        _eventBus.enqueue(
            InteractionModeChangeEvent{InteractionMode::CREATE_STATION});
        LOG_DEBUG("UI", "Interaction mode change requested: StationPlacement");
    }
    */
    ImGui::SameLine();
    if (ImGui::RadioButton("Line Creation", &currentMode,
                           static_cast<int>(InteractionMode::CREATE_LINE))) {
        _eventBus.enqueue(InteractionModeChangeEvent{InteractionMode::CREATE_LINE});
        LOG_DEBUG("UI", "Interaction mode change requested: LineCreation");
    }
    ImGui::End();
}

void UI::drawLineCreationWindow(size_t numberOfStationsInActiveLine) {
    if (_gameState.currentInteractionMode != InteractionMode::CREATE_LINE) {
        return;
    }

    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    ImGuiWindowFlags size_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                                  | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

    float interactionModesWidth = Constants::UI_INTERACTION_MODES_WIDTH;
    float interactionModesHeight = Constants::UI_INTERACTION_MODES_HEIGHT;
    ImVec2 interactionModesPos = ImVec2((displaySize.x - interactionModesWidth) * 0.5f,
                                        displaySize.y - interactionModesHeight - windowPadding);

    ImVec2 lineCreationWindowPos =
        ImVec2(interactionModesPos.x + ImGui::GetWindowWidth() + windowPadding,
               displaySize.y - interactionModesHeight - windowPadding);
    ImGui::SetNextWindowPos(lineCreationWindowPos, ImGuiCond_Always);
    ImGui::Begin("Line Creation", nullptr, size_flags);

    if (numberOfStationsInActiveLine < 2) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Finalize Line")) {
        _eventBus.enqueue<FinalizeLineEvent>({});
        LOG_DEBUG("UI", "Line finalization requested.");
    }
    if (numberOfStationsInActiveLine < 2) {
        ImGui::EndDisabled();
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel Line")) {
        _eventBus.enqueue<CancelLineCreationEvent>({});
    }
    ImGui::End();
}

void UI::drawSettingsWindow() {
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

void UI::onThemeChanged(const ThemeChangedEvent &event) {
    if (event.theme == Theme::Light) {
        ImGui::StyleColorsLight();
    } else {
        ImGui::StyleColorsDark();
    }
}

void UI::drawInfoPanel() {
    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    float worldGenSettingsWidth = Constants::UI_WORLD_GEN_SETTINGS_WIDTH;
    ImVec2 worldGenSettingsPos =
        ImVec2(displaySize.x - worldGenSettingsWidth - windowPadding, windowPadding);
    ImGui::SetNextWindowPos(ImVec2(worldGenSettingsPos.x, ImGui::GetFrameHeightWithSpacing() * 20),
                            ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(worldGenSettingsWidth, 0.0f), ImGuiCond_Always);
    ImGui::Begin("Info Panel", nullptr, window_flags);

    auto &selectedEntityOpt = _gameState.selectedEntity;
    if (!selectedEntityOpt.has_value()) {
        ImGui::Text("No information available.");
    } else {
        auto &registry = _game.getRegistry();
        auto entity = selectedEntityOpt.value();

        if (registry.valid(entity)) {
            if (auto *name = registry.try_get<NameComponent>(entity)) {
                ImGui::Text("Name: %s", name->name.c_str());
            }

            if (auto *city = registry.try_get<CityComponent>(entity)) {
                ImGui::Text("Type: City");
                ImGui::Text("Connected Lines: %zu", city->connectedLines.size());
                ImGui::Text("Waiting Passengers: %zu", city->waitingPassengers.size());
                if (ImGui::Button("Create Passenger")) {
                    _gameState.passengerOriginStation = entity;
                    _eventBus.enqueue<InteractionModeChangeEvent>(
                        {InteractionMode::CREATE_PASSENGER});
                }

                if (ImGui::CollapsingHeader("Waiting Passengers")) {
                    if (city->waitingPassengers.empty()) {
                        ImGui::Text("No passengers waiting.");
                    } else {
                        for (entt::entity passengerEntity : city->waitingPassengers) {
                            if (!registry.valid(passengerEntity)) continue;

                            auto &passenger = registry.get<PassengerComponent>(passengerEntity);
                            auto destinationStation = passenger.destinationStation;

                            std::string destinationName = "Unknown";
                            if (registry.valid(destinationStation)) {
                                if (auto *name =
                                        registry.try_get<NameComponent>(destinationStation)) {
                                    destinationName = name->name;
                                }
                            }

                            std::string label = "Passenger "
                                                + std::to_string(entt::to_integral(passengerEntity))
                                                + " -> " + destinationName;
                            if (ImGui::Selectable(label.c_str())) {
                                _gameState.selectedEntity = passengerEntity;
                            }
                        }
                    }
                }
            } else if (registry.all_of<TrainTag>(entity)) {
                auto &movement = registry.get<TrainMovementComponent>(entity);
                auto &physics = registry.get<TrainPhysicsComponent>(entity);
                auto &capacity = registry.get<TrainCapacityComponent>(entity);

                ImGui::Text("Type: Train");
                ImGui::Text("Assigned Line: %u", entt::to_integral(movement.assignedLine));
                const char *state = trainStateToString(movement.state);
                ImGui::Text("State: %s", state);
                ImGui::Text("Passengers: %d/%d", capacity.currentLoad, capacity.capacity);

                if (ImGui::Button("Delete Train")) {
                    _eventBus.enqueue<DeleteEntityEvent>({entity});
                    LOG_DEBUG("UI", "Delete train %u requested.", entt::to_integral(entity));
                }

                // The logic for listing passengers on a train needs to be updated
                // to query passengers by their currentContainer.
                if (ImGui::CollapsingHeader("Passengers")) {
                    auto passengerView = registry.view<PassengerComponent>();
                    bool foundPassengers = false;
                    for (auto passengerEntity : passengerView) {
                        auto& passenger = passengerView.get<PassengerComponent>(passengerEntity);
                        if (passenger.currentContainer == entity) {
                            foundPassengers = true;
                            auto destinationStation = passenger.destinationStation;

                            std::string destinationName = "Unknown";
                            if (registry.valid(destinationStation)) {
                                if (auto* name = registry.try_get<NameComponent>(destinationStation)) {
                                    destinationName = name->name;
                                }
                            }

                            std::string label = "Passenger " + std::to_string(entt::to_integral(passengerEntity)) + " -> " + destinationName;
                            if (ImGui::Selectable(label.c_str())) {
                                _gameState.selectedEntity = passengerEntity;
                            }
                        }
                    }

                    if (!foundPassengers) {
                        ImGui::Text("No passengers on board.");
                    }
                }
            } else if (auto *line = registry.try_get<LineComponent>(entity)) {
                ImGui::Text("Type: Line");
                ImGui::Text("Stops: %zu", line->stops.size());

                // Color Picker
                float color[4] = {line->color.r / 255.f, line->color.g / 255.f,
                                  line->color.b / 255.f, line->color.a / 255.f};
                if (ImGui::ColorEdit4("Color", color)) {
                    line->color.r = static_cast<std::uint8_t>(color[0] * 255);
                    line->color.g = static_cast<std::uint8_t>(color[1] * 255);
                    line->color.b = static_cast<std::uint8_t>(color[2] * 255);
                    line->color.a = static_cast<std::uint8_t>(color[3] * 255);
                }

                if (ImGui::Button("Add Train")) {
                    _eventBus.enqueue<AddTrainToLineEvent>({entity});
                    LOG_DEBUG("UI", "Add train to line %u requested.", entt::to_integral(entity));
                }

                ImGui::SameLine();
                if (ImGui::Button("Delete Line")) {
                    _eventBus.enqueue<DeleteEntityEvent>({entity});
                    LOG_DEBUG("UI", "Delete line %u requested.", entt::to_integral(entity));
                }

                // Find all trains on this line
                std::vector<entt::entity> trainsOnLine;
                auto trainView = registry.view<TrainTag, TrainMovementComponent>();
                for (auto trainEntity : trainView) {
                    auto &movement = trainView.get<TrainMovementComponent>(trainEntity);
                    if (movement.assignedLine == entity) {
                        trainsOnLine.push_back(trainEntity);
                    }
                }

                ImGui::Text("Train Count: %zu", trainsOnLine.size());

                if (ImGui::CollapsingHeader("Trains on Line")) {
                    if (trainsOnLine.empty()) {
                        ImGui::Text("No trains on this line.");
                    } else {
                        for (auto trainEntity : trainsOnLine) {
                            auto &movement = registry.get<TrainMovementComponent>(trainEntity);
                            auto *trainName = registry.try_get<NameComponent>(trainEntity);

                            std::string trainLabel =
                                trainName
                                    ? trainName->name
                                    : "Train " + std::to_string(entt::to_integral(trainEntity));

                            std::string location;
                            if (movement.state == TrainState::STOPPED) {
                                entt::entity currentStopEntity = entt::null;
                                if (movement.currentSegmentIndex < line->stops.size()) {
                                    currentStopEntity = line->stops[movement.currentSegmentIndex];
                                }

                                if (registry.valid(currentStopEntity)) {
                                    auto *stationName =
                                        registry.try_get<NameComponent>(currentStopEntity);
                                    location =
                                        "At "
                                        + (stationName ? stationName->name : "Unknown Station");
                                } else {
                                    location = "At an unknown station";
                                }
                            } else {
                                if (movement.currentSegmentIndex < line->stops.size() - 1) {
                                    entt::entity stop1_entity =
                                        line->stops[movement.currentSegmentIndex];
                                    entt::entity stop2_entity =
                                        line->stops[movement.currentSegmentIndex + 1];

                                    if (registry.valid(stop1_entity)
                                        && registry.valid(stop2_entity)) {
                                        auto *name1 = registry.try_get<NameComponent>(stop1_entity);
                                        auto *name2 = registry.try_get<NameComponent>(stop2_entity);

                                        std::string station1Name = name1 ? name1->name : "Unknown";
                                        std::string station2Name = name2 ? name2->name : "Unknown";

                                        if (movement.direction == TrainDirection::FORWARD) {
                                            location =
                                                "Between " + station1Name + " and " + station2Name;
                                        } else {
                                            location =
                                                "Between " + station2Name + " and " + station1Name;
                                        }
                                    } else {
                                        location = "Between unknown stations";
                                    }
                                } else {
                                    location = "In transit";
                                }
                            }

                            std::string fullLabel = trainLabel + " (" + location + ")";
                            if (ImGui::Selectable(fullLabel.c_str())) {
                                _gameState.selectedEntity = trainEntity;
                                LOG_DEBUG("UI", "Train %u selected from line info panel.",
                                          entt::to_integral(trainEntity));
                            }
                        }
                    }
                }

                if (ImGui::CollapsingHeader("Stops")) {
                    if (line->stops.empty()) {
                        ImGui::Text("This line has no stops.");
                    } else {
                        for (size_t i = 0; i < line->stops.size(); ++i) {
                            entt::entity stopEntity = line->stops[i];
                            if (registry.valid(stopEntity)) {
                                auto *name = registry.try_get<NameComponent>(stopEntity);
                                std::string stopName =
                                    name ? name->name
                                         : "Stop " + std::to_string(entt::to_integral(stopEntity));
                                std::string label = std::to_string(i + 1) + ". " + stopName;
                                if (ImGui::Selectable(label.c_str())) {
                                    _gameState.selectedEntity = stopEntity;
                                    LOG_DEBUG("UI", "Stop %u selected from line info panel.",
                                              entt::to_integral(stopEntity));
                                }
                            }
                        }
                    }
                }

            } else if (auto *passenger = registry.try_get<PassengerComponent>(entity)) {
                ImGui::Text("Type: Passenger");

                std::string originName = "Unknown";
                if (registry.valid(passenger->originStation)) {
                    if (auto *name = registry.try_get<NameComponent>(passenger->originStation)) {
                        originName = name->name;
                    }
                }
                ImGui::Text("Origin: %s", originName.c_str());

                std::string destinationName = "Unknown";
                if (registry.valid(passenger->destinationStation)) {
                    if (auto *name =
                            registry.try_get<NameComponent>(passenger->destinationStation)) {
                        destinationName = name->name;
                    }
                }
                ImGui::Text("Destination: %s", destinationName.c_str());

                const char *state;
                switch (passenger->state) {
                case PassengerState::WAITING_FOR_TRAIN:
                    state = "Waiting for train";
                    break;
                case PassengerState::ON_TRAIN:
                    state = "On train";
                    break;
                case PassengerState::ARRIVED:
                    state = "Arrived";
                    break;
                default:
                    state = "Unknown";
                    break;
                }
                ImGui::Text("State: %s", state);

                // Add button to toggle path visualization
                bool isVisualizing = registry.all_of<VisualizePathComponent>(entity);
                const char *buttonText = isVisualizing ? "Hide Path" : "Show Path";
                if (ImGui::Button(buttonText)) {
                    // Clear any existing visualization components first
                    auto view = registry.view<VisualizePathComponent>();
                    for (auto otherEntity : view) {
                        registry.remove<VisualizePathComponent>(otherEntity);
                    }

                    // If the button was "Show Path", add the component to the current entity
                    if (!isVisualizing) {
                        registry.emplace<VisualizePathComponent>(entity);
                    }
                }
            }
        } else {
            ImGui::Text("No information available.");
            selectedEntityOpt = std::nullopt;  // The entity is no longer valid
        }
    }

    ImGui::End();
}

void UI::drawPassengerCreationWindow() {
    if (_gameState.currentInteractionMode != InteractionMode::CREATE_PASSENGER) {
        return;
    }

    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    ImGuiWindowFlags size_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                                  | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

    float interactionModesWidth = Constants::UI_INTERACTION_MODES_WIDTH;
    float interactionModesHeight = Constants::UI_INTERACTION_MODES_HEIGHT;
    ImVec2 interactionModesPos = ImVec2((displaySize.x - interactionModesWidth) * 0.5f,
                                        displaySize.y - interactionModesHeight - windowPadding);

    ImVec2 passengerCreationWindowPos =
        ImVec2(interactionModesPos.x + ImGui::GetWindowWidth() + windowPadding,
               displaySize.y - interactionModesHeight - windowPadding);
    ImGui::SetNextWindowPos(passengerCreationWindowPos, ImGuiCond_Always);
    ImGui::Begin("Passenger Creation", nullptr, size_flags);

    ImGui::Text("Select a destination city for the new passenger.");

    if (ImGui::Button("Cancel")) {
        _eventBus.enqueue<InteractionModeChangeEvent>({InteractionMode::SELECT});
    }
    ImGui::End();
}

void UI::drawTimeControlWindow() {
    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::SetNextWindowPos(ImVec2(windowPadding, windowPadding));
    ImGui::Begin("Time Controls", nullptr, flags);

    float currentMultiplier = _gameState.timeMultiplier;
    ImVec4 activeColor = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];

    // Pause Button
    if (currentMultiplier == 0.0f) ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
    if (ImGui::Button("||")) _gameState.timeMultiplier = 0.0f;
    if (currentMultiplier == 0.0f) ImGui::PopStyleColor();

    ImGui::SameLine();

    // 1x Button
    if (currentMultiplier == 1.0f) ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
    if (ImGui::Button("1x")) _gameState.timeMultiplier = 1.0f;
    if (currentMultiplier == 1.0f) ImGui::PopStyleColor();

    ImGui::SameLine();

    // 2x Button
    if (currentMultiplier == 2.0f) ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
    if (ImGui::Button("2x")) _gameState.timeMultiplier = 2.0f;
    if (currentMultiplier == 2.0f) ImGui::PopStyleColor();

    ImGui::SameLine();

    // 3x Button
    if (currentMultiplier == 3.0f) ImGui::PushStyleColor(ImGuiCol_Button, activeColor);
    if (ImGui::Button("3x")) _gameState.timeMultiplier = 3.0f;
    if (currentMultiplier == 3.0f) ImGui::PopStyleColor();

    ImGui::End();
}