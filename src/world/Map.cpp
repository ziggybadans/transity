#include "Map.h"

void Map::SelectObject(sf::Vector2f pos) {
    // Attempt to select a Train first
    bool trainSelected = SelectTrain(pos);
    if (trainSelected) {
        DEBUG_DEBUG("Train selected.");
        return;
    }

    // If no Train was selected, attempt to select a Line handle
    if (SelectLineHandle(pos)) {
        DEBUG_DEBUG("Line handle selected.");
        return;
    }

    // If no Handle was selected, attempt to select a Line
    bool lineSelected = SelectLine(pos);
    if (lineSelected) {
        DEBUG_DEBUG("Line selected.");
        return;
    }

    // If no Line was selected, attempt to select a City
    bool citySelected = SelectCity(pos);
    if (citySelected) {
        DEBUG_DEBUG("City selected.");
        return;
    }

    // If no object was selected, deselect any currently selected objects
    DeselectAll();
    DEBUG_DEBUG("No object selected. All selections cleared.");
}

void Map::DeselectAll() {
    DeselectTrain();
    DeselectLine();
    DeselectCity();
}

// Tile management
void Map::SetTile(unsigned int x, unsigned int y, int value) {
    if (x >= m_size || y >= m_size)
        throw std::out_of_range("Invalid tile coordinates");
    m_grid[x][y] = value;
}

// City management
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
        sf::Vector2f diff = city.position - pos;
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

void Map::SelectCity(City* city) {
    if (selectedCity != nullptr) {
        // Implement if you have a selectedCity pointer
        selectedCity->selected = false;
    }

    selectedCity = city;

    if (selectedCity != nullptr) {
        selectedCity->selected = true;
        DeselectTrain();
        DeselectLine();
    }
}

bool Map::SelectCity(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 10.0f; // Adjust as needed

    for (auto& city : m_cities) {
        sf::Vector2f diff = city.position - pos;
        float distanceSquared = diff.x * diff.x + diff.y * diff.y;

        if (distanceSquared <= (city.radius + CLICK_THRESHOLD) * (city.radius + CLICK_THRESHOLD)) {
            SelectCity(&city);
            return true;
        }
    }

    DeselectCity();
    return false;
}

void Map::DeselectCity() {
    DEBUG_DEBUG("Any city has been deselected.");
    for (auto& city : m_cities) {
        city.selected = false;
    }
    selectedCity = nullptr;
}

// Line management
void Map::UseLineMode(sf::Vector2f pos) {
    DEBUG_DEBUG("Choosing to either create a new line or modify an existing line.");

    if (selectedLine == nullptr) {
        // No line is currently selected; create a new line starting at 'pos'
        CreateLine(pos);
    }
    else {
        if (!selectedLine->HasTrains()) {
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
                    AddToLineStart(clickedCity->position);
                }
                else if (selectedHandleIndex == static_cast<int>(selectedLine->GetPoints().size() - 1)) {
                    // Selected handle is the last node; add the city to the end
                    AddToLineEnd(clickedCity->position);
                }
                else {
                    // Selected handle is a middle node; insert the city after this handle
                    selectedLine->InsertCityAfter(selectedHandleIndex, clickedCity);
                }
            }
            else {
                // No specific handle is selected; default action is to add to the end
                AddToLineEnd(clickedCity->position);
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

    for (auto& city : m_cities) {
        sf::Vector2f diff = city.position - pos;
        float distanceSquared = diff.x * diff.x + diff.y * diff.y;
        if (distanceSquared <= static_cast<float>(m_minRadius * m_minRadius)) {
            firstCity = &city;
            break;
        }
    }

    if (firstCity == nullptr) {
        DEBUG_DEBUG("You need to click on a city to create a line!");
        return;
    }

    static int lineSuffix = 1;
    std::string name = "Line" + std::to_string(lineSuffix++);

    Line newLine(firstCity, name);

    m_lines.emplace_back(newLine);
    SelectLine(&m_lines.back());

    DEBUG_DEBUG("New line created originating from " + firstCity->name + " with name " + name + ". Selected line has been updated for new line.");
}

void Map::AddToLineStart(sf::Vector2f pos) {
    DEBUG_DEBUG("Adding city to the start of line " + selectedLine->GetName() + "...");

    City* newCity = FindCityAtPosition(pos);
    if (newCity == nullptr) {
        DEBUG_DEBUG("No valid city found at the clicked position to add to the line.");
        return;
    }

    // Check if the city is already part of the line
    auto cityList = selectedLine->GetCities();
    if (std::find(cityList.begin(), cityList.end(), newCity) != cityList.end()) {
        DEBUG_DEBUG("The city is already part of the line.");
        return;
    }

    selectedLine->AddCityToStart(newCity);
    DEBUG_DEBUG("Added city with name " + newCity->name + " to the start of line " + selectedLine->GetName());
}

void Map::AddToLineEnd(sf::Vector2f pos) {
    DEBUG_DEBUG("Adding city to the end of line " + selectedLine->GetName() + "...");

    City* newCity = FindCityAtPosition(pos);
    if (newCity == nullptr) {
        DEBUG_DEBUG("No valid city found at the clicked position to add to the line.");
        return;
    }

    // Check if the city is already part of the line
    auto cityList = selectedLine->GetCities();
    if (std::find(cityList.begin(), cityList.end(), newCity) != cityList.end()) {
        DEBUG_DEBUG("The city is already part of the line.");
        return;
    }

    selectedLine->AddCityToEnd(newCity);
    DEBUG_DEBUG("Added city with name " + newCity->name + " to the end of line " + selectedLine->GetName());
}

void Map::SelectLine(Line* line) {
    if (selectedLine != nullptr) {
        selectedLine->SetSelected(false);
    }

    selectedLine = line;

    if (selectedLine != nullptr) {
        selectedLine->SetSelected(true);
        DeselectTrain();
        DeselectCity();
    }
}

bool Map::SelectLine(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 5.0f; // Adjust as needed

    for (auto& line : m_lines) {
        // Iterate through all segments of the line
        const std::vector<BezierSegment>& segments = line.GetBezierSegments();
        for (const auto& segment : segments) {
            // Calculate distance from click position to the segment
            float distance = DistancePointToBezier(pos, segment);
            if (distance <= CLICK_THRESHOLD) {
                SelectLine(&line);
                return true;
            }
        }
    }

    DeselectLine();
    return false;
}

void Map::DeselectLine() {
    DEBUG_DEBUG("Any line has been deselected.");
    for (auto& line : m_lines) {
        line.SetSelected(false);
    }
    selectedLine = nullptr;
}

bool Map::SelectLineHandle(sf::Vector2f pos) {
    const float HANDLE_CLICK_THRESHOLD = 10.0f; // Adjust as needed

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

// Train management
void Map::AddTrain() {
    if (selectedLine == nullptr) {
        DEBUG_DEBUG("No line selected. Cannot add train.");
        return;
    }
    m_trains.emplace_back(selectedLine, 50.0f);
    selectedLine->AddTrain(&m_trains.back());
}

void Map::MoveSelectedLineHandle(sf::Vector2f newPos)
{
    if (selectedLine == nullptr) {
        DEBUG_DEBUG("MoveSelectedLineHandle: No line selected.");
        return;
    }

    if (!selectedLine->HasTrains()) {
        DEBUG_DEBUG("You need to remove any trains before you modify the line!");
        return;
    }

    int handleIndex = selectedLine->GetSelectedHandleIndex();
    if (handleIndex == -1) {
        DEBUG_DEBUG("MoveSelectedLineHandle: No handle is currently selected.");
        return;
    }

    // Ask the line to move the handle
    selectedLine->MoveHandle(handleIndex, newPos);
}

void Map::SelectTrain(Train* train) {
    if (selectedTrain != nullptr) {
        selectedTrain->SetSelected(false);
    }

    selectedTrain = train;

    if (selectedTrain != nullptr) {
        selectedTrain->SetSelected(true);
        DeselectLine();
        DeselectCity();
    }
}

bool Map::SelectTrain(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 10.0f; // Adjust as needed
    Train* closestTrain = nullptr;
    float closestDistanceSquared = CLICK_THRESHOLD * CLICK_THRESHOLD;

    for (auto& train : m_trains) {
        sf::Vector2f trainPos = train.GetPosition();
        sf::Vector2f diff = trainPos - pos;
        float distanceSquared = diff.x * diff.x + diff.y * diff.y;

        if (distanceSquared < closestDistanceSquared) {
            closestDistanceSquared = distanceSquared;
            closestTrain = &train;
        }
    }

    if (closestTrain != nullptr) {
        SelectTrain(closestTrain);
        return true;
    }
    else {
        DeselectTrain();
        return false;
    }
}

void Map::DeselectTrain() {
    DEBUG_DEBUG("Any train has been deselected.");
    for (auto& train : m_trains) {
        train.SetSelected(false);
    }
    selectedTrain = nullptr;
}

// Helper functions
void Map::Resize(unsigned int newSize) {
    m_grid.resize(newSize, std::vector<int>(newSize, 1));
    for (auto& row : m_grid) {
        row.resize(newSize, 1);
    }
    m_size = newSize;
}

float Map::DistancePointToBezier(sf::Vector2f point, const BezierSegment& segment) const
{
    // Sample points along the Bezier curve and find the minimum distance
    const int NUM_SAMPLES = 100;
    float minDistance = std::numeric_limits<float>::max();

    for (int i = 0; i <= NUM_SAMPLES; ++i) {
        float t = static_cast<float>(i) / NUM_SAMPLES;
        sf::Vector2f bezierPoint = ComputeCubicBezierPoint(segment, t);
        sf::Vector2f diff = point - bezierPoint;
        float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        if (distance < minDistance) {
            minDistance = distance;
        }
    }

    return minDistance;
}

sf::Vector2f Map::ComputeCubicBezierPoint(const BezierSegment& segment, float t) const
{
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    sf::Vector2f point = uuu * segment.start; // first term
    point += 3 * uu * t * segment.startControl; // second term
    point += 3 * u * tt * segment.endControl; // third term
    point += ttt * segment.end; // fourth term

    return point;
}

City* Map::FindCityAtPosition(sf::Vector2f pos)
{
    const float CLICK_THRESHOLD = 10.0f; // Adjust as needed

    for (auto& city : m_cities) {
        sf::Vector2f diff = city.position - pos;
        float distanceSquared = diff.x * diff.x + diff.y * diff.y;

        if (distanceSquared <= (city.radius + CLICK_THRESHOLD) * (city.radius + CLICK_THRESHOLD)) {
            return &city;
        }
    }

    return nullptr;
}
