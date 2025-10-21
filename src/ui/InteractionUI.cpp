#include "InteractionUI.h"
#include "Constants.h"
#include "Logger.h"
#include "app/InteractionMode.h"
#include "event/InputEvents.h"
#include "event/LineEvents.h"
#include "imgui.h"

InteractionUI::InteractionUI(GameState &gameState, EventBus &eventBus, sf::RenderWindow &window)
    : _gameState(gameState), _eventBus(eventBus), _window(window) {
    LOG_DEBUG("InteractionUI", "InteractionUI instance created.");
}

InteractionUI::~InteractionUI() {
    LOG_DEBUG("InteractionUI", "InteractionUI instance destroyed.");
}

void InteractionUI::draw(size_t numberOfStationsInActiveLine, size_t numberOfPointsInActiveLine,
                         std::optional<float> currentSegmentGrade,
                         bool currentSegmentExceedsGrade) {
    drawInteractionModeWindow();
    drawLineCreationWindow(numberOfStationsInActiveLine, numberOfPointsInActiveLine,
                           currentSegmentGrade, currentSegmentExceedsGrade);
    drawPassengerCreationWindow();
}

void InteractionUI::drawInteractionModeWindow() {
    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    ImGuiWindowFlags size_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                                  | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

    float interactionModesWidth = Constants::UI_INTERACTION_MODES_WIDTH;
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
    ImGui::SameLine();
    if (ImGui::RadioButton("Line Creation", &currentMode,
                           static_cast<int>(InteractionMode::CREATE_LINE))) {
        _eventBus.enqueue(InteractionModeChangeEvent{InteractionMode::CREATE_LINE});
        LOG_DEBUG("UI", "Interaction mode change requested: LineCreation");
    }
    ImGui::End();
}

void InteractionUI::drawLineCreationWindow(size_t numberOfStationsInActiveLine,
                                           size_t numberOfPointsInActiveLine,
                                           std::optional<float> currentSegmentGrade,
                                           bool currentSegmentExceedsGrade) {
    if (_gameState.currentInteractionMode != InteractionMode::CREATE_LINE) {
        return;
    }

    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    ImGuiWindowFlags size_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                                  | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

    float lineCreationWindowWidth = Constants::UI_LINE_CREATION_WINDOW_WIDTH;
    ImVec2 lineCreationWindowPos =
        ImVec2(displaySize.x - lineCreationWindowWidth - windowPadding,
               _window.getSize().y - ImGui::GetFrameHeightWithSpacing() * 3 - windowPadding);
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

    if (numberOfPointsInActiveLine == 0) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Cancel Line")) {
        _eventBus.enqueue<CancelLineCreationEvent>({});
    }
    if (numberOfPointsInActiveLine == 0) {
        ImGui::EndDisabled();
    }

    if (currentSegmentGrade.has_value() && numberOfPointsInActiveLine > 0) {
        ImGui::Separator();
        float gradePercent = currentSegmentGrade.value() * 100.0f;
        if (currentSegmentExceedsGrade) {
            ImGui::TextColored(ImVec4(0.95f, 0.3f, 0.3f, 1.0f), "Current grade: %.2f%%", gradePercent);
        } else {
            ImGui::Text("Current grade: %.2f%%", gradePercent);
        }
    }
    ImGui::End();
}

void InteractionUI::drawPassengerCreationWindow() {
    if (_gameState.currentInteractionMode != InteractionMode::CREATE_PASSENGER) {
        return;
    }

    const float windowPadding = Constants::UI_WINDOW_PADDING;
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 displaySize = io.DisplaySize;
    ImGuiWindowFlags size_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
                                  | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse;

    float interactionModesWidth = Constants::UI_INTERACTION_MODES_WIDTH;
    ImVec2 interactionModesPos =
        ImVec2((displaySize.x - interactionModesWidth) * 0.5f,
               displaySize.y - ImGui::GetFrameHeightWithSpacing() * 2.5 - windowPadding);

    ImVec2 passengerCreationWindowPos = ImVec2(
        interactionModesPos.x, interactionModesPos.y - ImGui::GetFrameHeightWithSpacing() * 2.0f);
    ImGui::SetNextWindowPos(passengerCreationWindowPos, ImGuiCond_Always);
    ImGui::Begin("Passenger Creation", nullptr, size_flags);

    ImGui::Text("Select a destination city for the new passenger.");

    if (ImGui::Button("Cancel")) {
        _eventBus.enqueue<InteractionModeChangeEvent>({InteractionMode::SELECT});
    }
    ImGui::End();
}
