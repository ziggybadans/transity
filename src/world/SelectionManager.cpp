#include "SelectionManager.h"
#include "../Debug.h"
#include "Map.h"

SelectionManager::SelectionManager(Map& map)
    : selectedCity(nullptr), selectedLine(nullptr), selectedTrain(nullptr), selectedNode(nullptr), m_map(map) {}

void SelectionManager::SelectObject(const sf::Vector2f& pos) {
    // Attempt to select a Train first
    if (SelectTrain(pos)) {
        DEBUG_DEBUG("Train selected.");
        return;
    }

    // Attempt to select a Line handle
    if (SelectLineHandle(pos)) {
        DEBUG_DEBUG("Line handle selected.");
        return;
    }

    // Attempt to select a Line
    if (SelectLine(pos)) {
        DEBUG_DEBUG("Line selected.");
        return;
    }

    // Attempt to select a City
    if (SelectCity(pos)) {
        DEBUG_DEBUG("City selected.");
        return;
    }

    // If no object was selected, deselect any currently selected objects
    DeselectAll();
    DEBUG_DEBUG("No object selected. All selections cleared.");
}

bool SelectionManager::SelectCity(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 10.0f; // Adjust as needed

    for (auto& city : m_map.GetCities()) {
        sf::Vector2f diff = city.GetPosition() - pos;
        float distanceSquared = diff.x * diff.x + diff.y * diff.y;

        if (distanceSquared <= (city.GetRadius() + CLICK_THRESHOLD) * (city.GetRadius() + CLICK_THRESHOLD)) {
            SelectCity(&city);
            return true;
        }
    }

    return false;
}

bool SelectionManager::SelectLineHandle(sf::Vector2f pos) {
    const float HANDLE_CLICK_THRESHOLD = 10.0f; // Adjust as needed

    Line* selectedLine = m_map.GetSelectionManager().GetSelectedLine();
    if (selectedLine == nullptr)
        return false;

    // Iterate through all handles
    for (const auto& handle : selectedLine->GetHandles()) {
        sf::Vector2f handlePos = selectedLine->GetPointPosition(handle.index);
        sf::Vector2f diff = handlePos - pos;
        float distanceSquared = diff.x * diff.x + diff.y * diff.y;

        if (distanceSquared <= HANDLE_CLICK_THRESHOLD * HANDLE_CLICK_THRESHOLD) {
            selectedLine->SelectHandle(handle.index);
            return true;
        }
    }

    // If no handle was clicked, deselect all handles
    selectedLine->DeselectHandles();
    return false;
}

bool SelectionManager::SelectLine(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 5.0f; // Adjust as needed
    float bestDistance = CLICK_THRESHOLD;
    Line* closestLine = nullptr;

    for (auto& line : m_map.GetLines()) {
        // Use the adjusted path points that account for offsets
        const auto& pathPoints = line.GetAdjustedPathPoints();

        // Iterate through all straight segments of the line
        for (size_t i = 0; i < pathPoints.size() - 1; ++i) {
            sf::Vector2f start = pathPoints[i];
            sf::Vector2f end = pathPoints[i + 1];

            // Calculate distance from click position to the segment
            float distance = m_map.DistancePointToSegment(pos, start, end);
            // Track the closest line if within threshold
            if (distance <= bestDistance) {
                bestDistance = distance;
                closestLine = &line;
            }
        }
    }

    if (closestLine) {
        SelectLine(closestLine);
        return true;
    }

    return false;
}

bool SelectionManager::SelectTrain(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 10.0f; // Adjust as needed
    Train* closestTrain = nullptr;
    float closestDistanceSquared = CLICK_THRESHOLD * CLICK_THRESHOLD;

    for (auto& train : m_map.GetTrains()) {
        sf::Vector2f trainPos = train->GetPosition();
        sf::Vector2f diff = trainPos - pos;
        float distanceSquared = diff.x * diff.x + diff.y * diff.y;

        if (distanceSquared < closestDistanceSquared) {
            closestDistanceSquared = distanceSquared;
            closestTrain = train.get();
        }
    }

    if (closestTrain != nullptr) {
        SelectTrain(closestTrain);
        return true;
    }
    else {
        return false;
    }
}

void SelectionManager::SelectCity(City* city) {
    DeselectAll();
    selectedCity = city;
    if (selectedCity) {
        selectedCity->SetSelected(true);
    }
}

void SelectionManager::SelectLine(Line* line) {
    DeselectAll();
    selectedLine = line;
    if (selectedLine) {
        selectedLine->SetSelected(true);
    }
}

void SelectionManager::SelectTrain(Train* train) {
    DeselectAll();
    selectedTrain = train;
    if (selectedTrain) {
        selectedTrain->SetSelected(true);
    }
}

void SelectionManager::SelectNode(Node* node) {
    DeselectAll();
    selectedNode = node;
    if (selectedNode) {
        selectedNode->SetSelected(true);
    }
}