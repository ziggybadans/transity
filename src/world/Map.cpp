// Map.cpp

#include "Map.h"

// Constructor
Map::Map(unsigned int size)
    : m_size(size),
    m_grid(size, std::vector<int>(size, 1)),
    m_minRadius(100),
    selectedLine(nullptr),
    selectedTrain(nullptr) {}

// In Map.cpp
void Map::SelectObject(sf::Vector2f pos) {
    // Attempt to select a Train first
    bool trainSelected = SelectTrain(pos);
    if (trainSelected) {
        DEBUG_DEBUG("Train selected.");
        return;
    }

    // If no Train was selected, attempt to select a Line
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

int Map::GetSize() const {
    return m_size;
}

int Map::GetTile(int x, int y) const {
    return m_grid[x][y];
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
        // selectedCity->SetSelected(false);
    }

    selectedCity = city;

    if (selectedCity != nullptr) {
        // Implement selection visualization if needed
        // DeselectTrain();
        // DeselectLine();
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
    DEBUG_DEBUG("Choosing to either create new line or add to existing line.");
    if (selectedLine == nullptr) {
        CreateLine(pos);
    }
    else {
        AddToLine(pos);
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

void Map::AddToLine(sf::Vector2f pos) {
    DEBUG_DEBUG("Adding city to line " + selectedLine->GetName() + "...");
    City* firstCity = nullptr;

    for (auto& city : m_cities) {
        sf::Vector2f diff = city.position - pos;
        float distanceSquared = diff.x * diff.x + diff.y * diff.y;
        if (distanceSquared <= static_cast<float>(m_minRadius * m_minRadius)) {
            firstCity = &city;
            break;
        }
    }

    if (firstCity == nullptr) {
        DEBUG_DEBUG("You need to click on a city to add one to the line!");
        return;
    }

    auto it = std::find(selectedLine->GetCities().begin(), selectedLine->GetCities().end(), firstCity);
    if (it != selectedLine->GetCities().end()) {
        DEBUG_DEBUG("You cannot add a city to the same line twice!");
        return;
    }

    selectedLine->AddCity(firstCity);
    DEBUG_DEBUG("Added city with name " + firstCity->name + " to line with name " + selectedLine->GetName());
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

// Train management
void Map::AddTrain() {
    if (selectedLine == nullptr) {
        DEBUG_DEBUG("No line selected. Cannot add train.");
        return;
    }
    Train newTrain(selectedLine, 50.0f); // speed = 50.0f (as an example)
    m_trains.push_back(newTrain);
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
