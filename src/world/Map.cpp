#include "Map.h"
#include "../Debug.h"

void Map::SelectObject(sf::Vector2f pos) {
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

void Map::DeselectAll() {
    selectionManager.DeselectAll();
}

void Map::SetTile(unsigned int x, unsigned int y, int value) {
    if (x >= m_size || y >= m_size)
        throw std::out_of_range("Invalid tile coordinates");
    m_grid[x][y] = value;
}

void Map::AddCity(sf::Vector2f pos) {
    /* Validation checks */
    // Check inside map bounds
    if (pos.x < 0 || pos.y < 0) { return; }
    if (pos.x >= static_cast<float>(m_size) * (Constants::TILE_SIZE * 0.98f) ||
        pos.y >= static_cast<float>(m_size) * (Constants::TILE_SIZE * 0.98f)) {
        return;
    }

    // Check outside minimum radius to another city
    for (const City& city : m_cities) {
        sf::Vector2f diff = city.GetPosition() - pos;
        float distanceSquared = diff.x * diff.x + diff.y * diff.y;
        if (distanceSquared <= static_cast<float>(m_minRadius * m_minRadius)) {
            return;
        }
    }

    /* Data generation */
    static int citySuffix = 1;
    std::string name = "City" + std::to_string(citySuffix++);
    unsigned int population = 1000;

    m_cities.emplace_back(name, pos, population); // Adds to the list of cities
}

bool Map::SelectCity(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 10.0f; // Adjust as needed

    for (auto& city : m_cities) {
        sf::Vector2f diff = city.GetPosition() - pos;
        float distanceSquared = diff.x * diff.x + diff.y * diff.y;

        if (distanceSquared <= (city.GetRadius() + CLICK_THRESHOLD) * (city.GetRadius() + CLICK_THRESHOLD)) {
            SelectCity(&city);
            return true;
        }
    }

    DeselectAll();
    return false;
}

void Map::UseLineMode(sf::Vector2f pos) {
    DEBUG_DEBUG("Choosing to either create a new line or modify an existing line.");

    Line* selectedLine = selectionManager.GetSelectedLine();

    if (selectedLine == nullptr) {
        // No line is currently selected; create a new line starting at 'pos'
        CreateLine(pos);
    }
    else {
        if (selectedLine->HasTrains()) {
            DEBUG_DEBUG("You need to remove any trains from the line before modifying it!");
            return;
        }

        // Attempt to find if a city was clicked
        City* clickedCity = FindCityAtPosition(pos);

        if (clickedCity) {
            // Retrieve the index of the selected handle (node)
            int selectedHandleIndex = selectedLine->GetSelectedHandleIndex();

            if (selectedHandleIndex != -1) {
                // Handle is selected; determine where to add the new city relative to it
                if (selectedHandleIndex == 0) {
                    // Selected handle is the first node; add the city to the start
                    AddToLineStart(clickedCity->GetPosition());
                }
                else if (selectedHandleIndex == static_cast<int>(selectedLine->GetPoints().size() - 1)) {
                    // Selected handle is the last node; add the city to the end
                    AddToLineEnd(clickedCity->GetPosition());
                }
                else {
                    // Selected handle is a middle node; insert the city after this handle
                    selectedLine->InsertCityAfter(selectedHandleIndex, clickedCity);
                }
            }
            else {
                // No specific handle is selected; default action is to add to the end
                AddToLineEnd(clickedCity->GetPosition());
            }
        }
        else {
            // No city was clicked; add a generic node at the clicked position
            selectedLine->AddNode(pos);
        }
    }
}

void Map::CreateLine(sf::Vector2f pos) {
    DEBUG_DEBUG("Creating new line...");
    City* firstCity = nullptr;

    if (m_cities.empty()) {
        DEBUG_DEBUG("You need to create a city first!");
        return;
    }

    firstCity = FindCityAtPosition(pos);

    if (firstCity == nullptr) {
        DEBUG_DEBUG("You need to click on a city to create a line!");
        return;
    }

    static int lineSuffix = 1;
    std::string name = "Line" + std::to_string(lineSuffix++);

    m_lines.emplace_back(firstCity, name);
    Line* newLine = &m_lines.back();
    SelectLine(newLine);

    DEBUG_DEBUG("New line created originating from " + firstCity->GetName() + " with name " + name + ". Selected line has been updated for new line.");
}

void Map::AddToLineStart(sf::Vector2f pos) {
    DEBUG_DEBUG("Adding city to the start of line " + GetSelectedLine()->GetName() + "...");

    City* newCity = FindCityAtPosition(pos);
    if (newCity == nullptr) {
        DEBUG_DEBUG("No valid city found at the clicked position to add to the line.");
        return;
    }

    // Check if the city is already part of the line
    auto cityList = GetSelectedLine()->GetCities();
    if (std::find(cityList.begin(), cityList.end(), newCity) != cityList.end()) {
        DEBUG_DEBUG("The city is already part of the line.");
        return;
    }

    GetSelectedLine()->AddCityToStart(newCity);
    DEBUG_DEBUG("Added city with name " + newCity->GetName() + " to the start of line " + GetSelectedLine()->GetName());
}

void Map::AddToLineEnd(sf::Vector2f pos) {
    DEBUG_DEBUG("Adding city to the end of line " + GetSelectedLine()->GetName() + "...");

    City* newCity = FindCityAtPosition(pos);
    if (newCity == nullptr) {
        DEBUG_DEBUG("No valid city found at the clicked position to add to the line.");
        return;
    }

    // Check if the city is already part of the line
    auto cityList = GetSelectedLine()->GetCities();
    if (std::find(cityList.begin(), cityList.end(), newCity) != cityList.end()) {
        DEBUG_DEBUG("The city is already part of the line.");
        return;
    }

    GetSelectedLine()->AddCityToEnd(newCity);
    DEBUG_DEBUG("Added city with name " + newCity->GetName() + " to the end of line " + GetSelectedLine()->GetName());
}

bool Map::SelectLine(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 5.0f; // Adjust as needed

    for (auto& line : m_lines) {
        const auto& pathPoints = line.GetPathPoints();

        // Iterate through all straight segments of the line
        for (size_t i = 0; i < pathPoints.size() - 1; ++i) {
            sf::Vector2f start = pathPoints[i];
            sf::Vector2f end = pathPoints[i + 1];

            // Calculate distance from click position to the segment
            float distance = DistancePointToSegment(pos, start, end);
            if (distance <= CLICK_THRESHOLD) {
                SelectLine(&line);
                return true;
            }
        }
    }

    DeselectAll();
    return false;
}

bool Map::SelectLineHandle(sf::Vector2f pos) {
    const float HANDLE_CLICK_THRESHOLD = 10.0f; // Adjust as needed

    Line* selectedLine = selectionManager.GetSelectedLine();
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

void Map::RemoveLine() {
    Line* selectedLine = selectionManager.GetSelectedLine();
    if (selectedLine == nullptr) {
        DEBUG_DEBUG("No line selected. Cannot remove line.");
        return;
    }

    if (selectedLine->HasTrains()) {
        DEBUG_DEBUG("Cannot remove line. It has trains assigned.");
        return;
    }

    // Find the iterator to the selected line
    auto it = std::find_if(m_lines.begin(), m_lines.end(),
        [selectedLine](const Line& line) { return &line == selectedLine; });

    if (it != m_lines.end()) {
        DEBUG_DEBUG("Removing line: " + it->GetName());
        m_lines.erase(it);
        selectionManager.DeselectAll();
        DEBUG_DEBUG("Line removed successfully.");
    }
    else {
        DEBUG_DEBUG("Selected line not found in the lines list.");
    }
}

void Map::MoveSelectedLineHandle(sf::Vector2f newPos) {
    Line* selectedLine = selectionManager.GetSelectedLine();
    if (selectedLine == nullptr) {
        DEBUG_DEBUG("MoveSelectedLineHandle: No line selected.");
        return;
    }

    if (selectedLine->HasTrains()) {
        DEBUG_DEBUG("You need to remove any trains before you modify the line!");
        return;
    }

    int handleIndex = selectedLine->GetSelectedHandleIndex();
    if (handleIndex == -1) {
        DEBUG_DEBUG("MoveSelectedLineHandle: No handle is currently selected.");
        return;
    }

    // Move the handle
    selectedLine->MoveHandle(handleIndex, newPos);
}

void Map::AddTrain() {
    Line* selectedLine = selectionManager.GetSelectedLine();
    if (selectedLine == nullptr) {
        DEBUG_DEBUG("No line selected. Cannot add train.");
        return;
    }

    static int trainSuffix = 1;
    std::string name = "Train" + std::to_string(trainSuffix++);

    // Create a new Train object and add it to the vector
    auto newTrain = std::make_unique<Train>(selectedLine, name, 50.0f);
    Train* trainPtr = newTrain.get(); // Get raw pointer for Line reference
    m_trains.emplace_back(std::move(newTrain));
    selectedLine->AddTrain(trainPtr);
}

bool Map::SelectTrain(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 10.0f; // Adjust as needed
    Train* closestTrain = nullptr;
    float closestDistanceSquared = CLICK_THRESHOLD * CLICK_THRESHOLD;

    for (auto& train : m_trains) {
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
        DeselectAll();
        return false;
    }
}

void Map::RemoveTrain() {
    Train* selectedTrain = selectionManager.GetSelectedTrain();
    if (!selectedTrain) { return; }

    // Remove train from the line
    Line* route = selectedTrain->GetRoute();
    if (route) {
        route->RemoveTrain(selectedTrain);
    }

    // Erase from the vector
    m_trains.erase(
        std::remove_if(m_trains.begin(), m_trains.end(),
            [selectedTrain](const std::unique_ptr<Train>& tptr) {
                return tptr.get() == selectedTrain;
            }
        ),
        m_trains.end()
    );

    // Deselect train
    selectionManager.DeselectAll();
}

City* Map::FindCityAtPosition(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 10.0f; // Adjust as needed

    for (auto& city : m_cities) {
        sf::Vector2f diff = city.GetPosition() - pos;
        float distanceSquared = diff.x * diff.x + diff.y * diff.y;

        if (distanceSquared <= (city.GetRadius() + CLICK_THRESHOLD) * (city.GetRadius() + CLICK_THRESHOLD)) {
            return &city;
        }
    }

    return nullptr;
}

float Map::DistancePointToSegment(const sf::Vector2f& point, const sf::Vector2f& segStart, const sf::Vector2f& segEnd) {
    sf::Vector2f seg = segEnd - segStart;
    sf::Vector2f pt = point - segStart;
    float segLengthSquared = seg.x * seg.x + seg.y * seg.y;

    if (segLengthSquared == 0.0f)
        return std::sqrt(pt.x * pt.x + pt.y * pt.y); // segStart and segEnd are the same point

    float t = (pt.x * seg.x + pt.y * seg.y) / segLengthSquared;
    t = std::max(0.0f, std::min(1.0f, t));
    sf::Vector2f projection = segStart + t * seg;
    sf::Vector2f diff = point - projection;
    return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}
