#include "UIManager.h"
#include <string> // For std::strncpy
#include <stdio.h>
#include "../core/Station.h" // Include Station class

/**
<summary>
UIManager is responsible for managing the ImGui-based user interface of the application.
It interacts with the WorldMap and provides UI components for modifying game elements such as line properties.
</summary>
*/
UIManager::UIManager(std::shared_ptr<WorldMap> worldMap)
    : initialized(false), renderWindow(nullptr), worldMap(worldMap), timeScalePtr(nullptr)
{
    // Initialize default color and thickness
    thickness = 2.0f;
    color[0] = 0.0f; // Red
    color[1] = 0.0f; // Green
    color[2] = 1.0f; // Blue
    color[3] = 1.0f; // Alpha
}

/**
<summary>
Destructor for UIManager. Ensures that ImGui is properly shut down before the object is destroyed.
</summary>
*/
UIManager::~UIManager() {
    Shutdown();
}

void UIManager::SetTimeScalePointer(std::atomic<float>* ptr) {
    timeScalePtr = ptr;
}

/**
<summary>
Sets the render window that the UI will be drawn on.
</summary>
<param name="window">Reference to the SFML RenderWindow to be used for rendering ImGui.</param>
*/
void UIManager::SetWindow(sf::RenderWindow& window) {
    renderWindow = &window;
}

/**
<summary>
Initializes ImGui for use with the provided render window.
</summary>
<returns>True if ImGui was successfully initialized, otherwise false.</returns>
*/
bool UIManager::Init() {
    if (renderWindow) {
        ImGui::SFML::Init(*renderWindow);
        initialized = true;
        return true;
    }
    return false;
}

/**
<summary>
Processes an SFML event, forwarding it to ImGui for handling.
</summary>
<param name="event">Reference to the SFML Event object to be processed by ImGui.</param>
*/
void UIManager::ProcessEvent(const sf::Event& event) {
    if (initialized) {
        ImGui::SFML::ProcessEvent(event);
    }
}

/**
<summary>
Updates ImGui with the current frame's delta time, allowing it to handle animations and UI interactions.
</summary>
<param name="deltaTime">Time elapsed since the last frame, in seconds.</param>
*/
void UIManager::Update(float deltaTime) {
    if (initialized && renderWindow) {
        ImGui::SFML::Update(*renderWindow, sf::seconds(deltaTime));
    }
}

/**
<summary>
Renders the ImGui components to the screen, such as line properties for the selected line in the WorldMap.
</summary>
*/
void UIManager::Render() {
    if (initialized && renderWindow) {
        Line* selectedLine = worldMap->GetSelectedLine();
        if (selectedLine) {
            ImGui::Begin("Line Properties");

            // Retrieve current properties
            sf::Color lineColor = selectedLine->GetColor();
            color[0] = lineColor.r / 255.0f;
            color[1] = lineColor.g / 255.0f;
            color[2] = lineColor.b / 255.0f;
            color[3] = lineColor.a / 255.0f;
            thickness = selectedLine->GetThickness();

            // Color picker
            if (ImGui::ColorEdit4("Line Color", color)) {
                // Update the selected line's color
                sf::Color newColor(
                    static_cast<sf::Uint8>(color[0] * 255),
                    static_cast<sf::Uint8>(color[1] * 255),
                    static_cast<sf::Uint8>(color[2] * 255),
                    static_cast<sf::Uint8>(color[3] * 255)
                );
                selectedLine->SetColor(newColor);
            }

            // Thickness slider
            if (ImGui::SliderFloat("Line Thickness", &thickness, 1.0f, 10.0f)) {
                selectedLine->SetThickness(thickness);
            }

            // Speed control
            speedKmPerHour = selectedLine->GetSpeed();
            if (ImGui::InputFloat("Speed (km/h)", &speedKmPerHour)) {
                selectedLine->SetSpeed(speedKmPerHour);
            }

            // Button to add a train
            if (ImGui::Button("Add Train")) {
                selectedLine->AddTrain();
            }

            if (ImGui::Button("Remove All Trains")) {
                selectedLine->RemoveTrains();
            }

            ImGui::End();
        }

        // Add station properties UI
        Station* selectedStation = worldMap->GetSelectedStation();
        if (selectedStation) {
            ImGui::Begin("Station Properties");

            // Get current name
            char nameBuffer[128];
            strncpy_s(nameBuffer, selectedStation->GetName().c_str(), sizeof(nameBuffer));
            nameBuffer[sizeof(nameBuffer) - 1] = '\0';

            if (ImGui::InputText("Station Name", nameBuffer, sizeof(nameBuffer))) {
                selectedStation->SetName(nameBuffer);
            }

            ImGui::End();
        }

        if (timeScalePtr) {
            ImGui::Begin("Time Control");

            // Display current time scale
            float currentScale = timeScalePtr->load();
            ImGui::Text("Current Time Scale: %.1fx", currentScale);

            // Slider for time scale
            if (ImGui::SliderFloat("Adjust Time Scale", &currentScale, 0.0f, 4.0f)) {
                timeScalePtr->store(currentScale);
            }

            // Preset speed buttons
            if (ImGui::Button("Pause")) {
                timeScalePtr->store(0.0f);
            }
            ImGui::SameLine();
            if (ImGui::Button("1x")) {
                timeScalePtr->store(1.0f);
            }
            ImGui::SameLine();
            if (ImGui::Button("2x")) {
                timeScalePtr->store(2.0f);
            }
            ImGui::SameLine();
            if (ImGui::Button("4x")) {
                timeScalePtr->store(4.0f);
            }

            ImGui::End();
        }

        ImGui::SFML::Render(*renderWindow);
    }
}

/**
<summary>
Shuts down ImGui and cleans up any allocated resources.
</summary>
*/
void UIManager::Shutdown() {
    if (initialized) {
        ImGui::SFML::Shutdown();
        initialized = false;
    }
}
