#include "InfoPanelUI.h"
#include "Constants.h"
#include "Logger.h"
#include "components/GameLogicComponents.h"
#include "components/LineComponents.h"
#include "components/PassengerComponents.h"
#include "components/RenderComponents.h"
#include "components/TrainComponents.h"
#include "event/DeletionEvents.h"
#include "event/InputEvents.h"
#include "event/LineEvents.h"
#include "imgui.h"
#include <numeric>

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

InfoPanelUI::InfoPanelUI(entt::registry &registry, EventBus &eventBus, GameState &gameState)
    : _registry(registry), _eventBus(eventBus), _gameState(gameState),
      _selectedEntity(std::nullopt) {
    _entitySelectedConnection =
        _eventBus.sink<EntitySelectedEvent>().connect<&InfoPanelUI::onEntitySelected>(this);
    _entityDeselectedConnection =
        _eventBus.sink<EntityDeselectedEvent>().connect<&InfoPanelUI::onEntityDeselected>(this);
    LOG_DEBUG("InfoPanelUI", "InfoPanelUI instance created.");
}

InfoPanelUI::~InfoPanelUI() {
    LOG_DEBUG("InfoPanelUI", "InfoPanelUI instance destroyed.");
}

void InfoPanelUI::onEntitySelected(const EntitySelectedEvent &event) {
    _selectedEntity = event.entity;
}

void InfoPanelUI::onEntityDeselected(const EntityDeselectedEvent &event) {
    _selectedEntity = std::nullopt;
}

void InfoPanelUI::draw() {
    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    float worldGenSettingsWidth = Constants::UI_WORLD_GEN_SETTINGS_WIDTH;
    ImVec2 worldGenSettingsPos =
        ImVec2(displaySize.x - worldGenSettingsWidth - windowPadding, windowPadding);
    ImGui::SetNextWindowPos(ImVec2(worldGenSettingsPos.x, ImGui::GetFrameHeightWithSpacing() * 21),
                            ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(worldGenSettingsWidth, 0.0f), ImGuiCond_Always);
    ImGui::Begin("Info Panel", nullptr, window_flags);

    if (!_selectedEntity.has_value() || !_registry.valid(_selectedEntity.value())) {
        ImGui::Text("No information available.");
        if (_selectedEntity.has_value()) {
            _selectedEntity = std::nullopt;
        }
    } else {
        auto entity = _selectedEntity.value();
        if (auto *name = _registry.try_get<NameComponent>(entity)) {
            ImGui::Text("Name: %s", name->name.c_str());
        }

        if (_registry.all_of<CityComponent>(entity)) {
            drawCityInfo(entity);
        } else if (_registry.all_of<TrainTag>(entity)) {
            drawTrainInfo(entity);
        } else if (_registry.all_of<LineComponent>(entity)) {
            drawLineInfo(entity);
        } else if (_registry.all_of<PassengerComponent>(entity)) {
            drawPassengerInfo(entity);
        }
    }

    ImGui::End();
}

void InfoPanelUI::drawCityInfo(entt::entity entity) {
    auto &city = _registry.get<CityComponent>(entity);
    ImGui::Text("Type: City");
    ImGui::Text("Connected Lines: %zu", city.connectedLines.size());

    std::vector<entt::entity> waitingPassengers;
    auto passengerView = _registry.view<PassengerComponent>();
    for (auto passengerEntity : passengerView) {
        const auto &passenger = passengerView.get<PassengerComponent>(passengerEntity);
        if (passenger.currentContainer == entity
            && passenger.state == PassengerState::WAITING_FOR_TRAIN) {
            waitingPassengers.push_back(passengerEntity);
        }
    }

    ImGui::Text("Waiting Passengers: %zu", waitingPassengers.size());
    if (ImGui::Button("Create Passenger")) {
        _eventBus.enqueue<StartPassengerCreationEvent>({entity});
    }

    if (ImGui::CollapsingHeader("Waiting Passengers")) {
        if (waitingPassengers.empty()) {
            ImGui::Text("No passengers waiting.");
        } else {
            for (entt::entity passengerEntity : waitingPassengers) {
                if (!_registry.valid(passengerEntity)) continue;
                auto &passenger = _registry.get<PassengerComponent>(passengerEntity);
                std::string destName =
                    _registry.valid(passenger.destinationStation)
                            && _registry.try_get<NameComponent>(passenger.destinationStation)
                        ? _registry.get<NameComponent>(passenger.destinationStation).name
                        : "Unknown";
                std::string label = "Passenger "
                                    + std::to_string(entt::to_integral(passengerEntity)) + " -> "
                                    + destName;
                if (ImGui::Selectable(label.c_str())) {
                    _eventBus.enqueue<EntitySelectedEvent>({passengerEntity});
                }
            }
        }
    }
}

void InfoPanelUI::drawTrainInfo(entt::entity entity) {
    auto &movement = _registry.get<TrainMovementComponent>(entity);
    auto &capacity = _registry.get<TrainCapacityComponent>(entity);

    ImGui::Text("Type: Train");
    ImGui::Text("Assigned Line: %u", entt::to_integral(movement.assignedLine));
    ImGui::Text("State: %s", trainStateToString(movement.state));
    ImGui::Text("Passengers: %d/%d", capacity.currentLoad, capacity.capacity);

    if (ImGui::Button("Delete Train")) {
        _eventBus.enqueue<DeleteEntityEvent>({entity});
    }

    if (ImGui::CollapsingHeader("Passengers")) {
        auto passengerView = _registry.view<PassengerComponent>();
        bool foundPassengers = false;
        for (auto passengerEntity : passengerView) {
            auto &passenger = passengerView.get<PassengerComponent>(passengerEntity);
            if (passenger.currentContainer == entity) {
                foundPassengers = true;
                std::string destName = "Unknown";
                if (_registry.valid(passenger.destinationStation)
                    && _registry.try_get<NameComponent>(passenger.destinationStation)) {
                    destName = _registry.get<NameComponent>(passenger.destinationStation).name;
                }
                std::string label = "Passenger "
                                    + std::to_string(entt::to_integral(passengerEntity)) + " -> "
                                    + destName;
                if (ImGui::Selectable(label.c_str())) {
                    _eventBus.enqueue<EntitySelectedEvent>({passengerEntity});
                }
            }
        }
        if (!foundPassengers) {
            ImGui::Text("No passengers on board.");
        }
    }
}

void InfoPanelUI::drawLineInfo(entt::entity entity) {
    auto &line = _registry.get<LineComponent>(entity);
    ImGui::Text("Type: Line");
    size_t stopCount = std::accumulate(line.points.begin(), line.points.end(), 0,
                                       [](size_t acc, const LinePoint &p) {
                                           return acc + (p.type == LinePointType::STOP ? 1 : 0);
                                       });
    ImGui::Text("Stops: %zu", stopCount);

    float color[4] = {line.color.r / 255.f, line.color.g / 255.f, line.color.b / 255.f,
                      line.color.a / 255.f};
    if (ImGui::ColorEdit4("Color", color)) {
        line.color = sf::Color(color[0] * 255, color[1] * 255, color[2] * 255, color[3] * 255);
    }

    if (_gameState.currentInteractionMode == InteractionMode::EDIT_LINE) {
        if (ImGui::Button("Done")) {
            _eventBus.enqueue<InteractionModeChangeEvent>({InteractionMode::SELECT});
        }
    } else {
        if (ImGui::Button("Edit Line")) {
            _eventBus.enqueue<InteractionModeChangeEvent>({InteractionMode::EDIT_LINE});
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Add Train")) {
        _eventBus.enqueue<AddTrainToLineEvent>({entity});
    }

    ImGui::SameLine();
    if (ImGui::Button("Delete Line")) {
        _eventBus.enqueue<DeleteEntityEvent>({entity});
    }

    std::vector<entt::entity> trainsOnLine;
    auto trainView = _registry.view<TrainTag, TrainMovementComponent>();
    for (auto trainEntity : trainView) {
        if (trainView.get<TrainMovementComponent>(trainEntity).assignedLine == entity) {
            trainsOnLine.push_back(trainEntity);
        }
    }
    ImGui::Text("Train Count: %zu", trainsOnLine.size());

    if (ImGui::CollapsingHeader("Trains on Line")) {
        if (trainsOnLine.empty()) {
            ImGui::Text("No trains on this line.");
        } else {
            for (auto trainEntity : trainsOnLine) {
                auto &movement = _registry.get<TrainMovementComponent>(trainEntity);
                auto *trainName = _registry.try_get<NameComponent>(trainEntity);

                std::string trainLabel =
                    trainName ? trainName->name
                              : "Train " + std::to_string(entt::to_integral(trainEntity));

                std::string location;
                if (movement.state == TrainState::STOPPED) {
                    entt::entity currentStopEntity = entt::null;
                    float min_dist = -1.f;

                    for (const auto &stop : line.stops) {
                        float dist =
                            std::abs(stop.distanceAlongCurve - movement.distanceAlongCurve);
                        if (min_dist < 0 || dist < min_dist) {
                            min_dist = dist;
                            currentStopEntity = stop.stationEntity;
                        }
                    }

                    if (_registry.valid(currentStopEntity)) {
                        auto *stationName = _registry.try_get<NameComponent>(currentStopEntity);
                        location = "At " + (stationName ? stationName->name : "Unknown Station");
                    } else {
                        location = "At an unknown station";
                    }
                } else {
                    entt::entity nextStopEntity = entt::null;
                    float min_dist = -1.f;

                    if (movement.direction == TrainDirection::FORWARD) {
                        for (const auto &stop : line.stops) {
                            if (stop.distanceAlongCurve > movement.distanceAlongCurve) {
                                float dist = stop.distanceAlongCurve - movement.distanceAlongCurve;
                                if (min_dist < 0 || dist < min_dist) {
                                    min_dist = dist;
                                    nextStopEntity = stop.stationEntity;
                                }
                            }
                        }
                    } else {  // BACKWARD
                        for (const auto &stop : line.stops) {
                            if (stop.distanceAlongCurve < movement.distanceAlongCurve) {
                                float dist = movement.distanceAlongCurve - stop.distanceAlongCurve;
                                if (min_dist < 0 || dist < min_dist) {
                                    min_dist = dist;
                                    nextStopEntity = stop.stationEntity;
                                }
                            }
                        }
                    }

                    if (_registry.valid(nextStopEntity)) {
                        auto *name = _registry.try_get<NameComponent>(nextStopEntity);
                        location = "Towards " + (name ? name->name : "Unknown");
                    } else {
                        location = "In transit";
                    }
                }

                std::string fullLabel = trainLabel + " (" + location + ")";
                if (ImGui::Selectable(fullLabel.c_str())) {
                    _eventBus.enqueue<EntitySelectedEvent>({trainEntity});
                }
            }
        }
    }

    if (ImGui::CollapsingHeader("Stops")) {
        if (stopCount == 0) {
            ImGui::Text("This line has no stops.");
        } else {
            int stopNum = 1;
            for (const auto &point : line.points) {
                if (point.type == LinePointType::STOP) {
                    entt::entity stopEntity = point.stationEntity;
                    if (_registry.valid(stopEntity)) {
                        auto *name = _registry.try_get<NameComponent>(stopEntity);
                        std::string stopName =
                            name ? name->name
                                 : "Stop " + std::to_string(entt::to_integral(stopEntity));
                        std::string label = std::to_string(stopNum++) + ". " + stopName;
                        if (ImGui::Selectable(label.c_str())) {
                            _eventBus.enqueue<EntitySelectedEvent>({stopEntity});
                        }
                    }
                }
            }
        }
    }
}

void InfoPanelUI::drawPassengerInfo(entt::entity entity) {
    auto &passenger = _registry.get<PassengerComponent>(entity);
    ImGui::Text("Type: Passenger");

    std::string originName = "Unknown";
    if (_registry.valid(passenger.originStation)
        && _registry.try_get<NameComponent>(passenger.originStation)) {
        originName = _registry.get<NameComponent>(passenger.originStation).name;
    }
    ImGui::Text("Origin: %s", originName.c_str());

    std::string destName = "Unknown";
    if (_registry.valid(passenger.destinationStation)
        && _registry.try_get<NameComponent>(passenger.destinationStation)) {
        destName = _registry.get<NameComponent>(passenger.destinationStation).name;
    }
    ImGui::Text("Destination: %s", destName.c_str());

    const char *state;
    switch (passenger.state) {
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

    bool isVisualizing = _registry.all_of<VisualizePathComponent>(entity);
    if (ImGui::Button(isVisualizing ? "Hide Path" : "Show Path")) {
        auto view = _registry.view<VisualizePathComponent>();
        _registry.remove<VisualizePathComponent>(view.begin(), view.end());
        if (!isVisualizing) {
            _registry.emplace<VisualizePathComponent>(entity);
        }
    }
}