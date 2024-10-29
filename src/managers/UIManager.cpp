#include "UIManager.h"

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

UIManager::~UIManager() {
    Shutdown();
}

void UIManager::SetWindow(sf::RenderWindow& window) {
    renderWindow = &window;
}

bool UIManager::Init() {
    if (renderWindow) {
        ImGui::SFML::Init(*renderWindow);
        initialized = true;
        return true;
    }
    return false;
}

void UIManager::ProcessEvent(const sf::Event& event) {
    if (initialized) {
        ImGui::SFML::ProcessEvent(event);
    }
}

void UIManager::Update(float deltaTime) {
    if (initialized && renderWindow) {
        ImGui::SFML::Update(*renderWindow, sf::seconds(deltaTime));
    }
}

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

            ImGui::End();
        }

        ImGui::SFML::Render(*renderWindow);
    }
}

void UIManager::Shutdown() {
    if (initialized) {
        ImGui::SFML::Shutdown();
        initialized = false;
    }
}
