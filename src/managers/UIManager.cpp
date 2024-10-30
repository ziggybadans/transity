#include "UIManager.h"

/**
<summary>
UIManager is responsible for managing the ImGui-based user interface of the application.
It interacts with the WorldMap and provides UI components for modifying game elements such as line properties.
</summary>
*/
UIManager::UIManager(std::shared_ptr<WorldMap> worldMap)
    : initialized(false), renderWindow(nullptr), worldMap(worldMap)
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

            // Button to add a train
            if (ImGui::Button("Add Train")) {
                selectedLine->AddTrain();
            }

            if (ImGui::Button("Remove All Trains")) {
                selectedLine->RemoveTrains();
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
