// Map.cpp

#include "Map.h"

// Constructor
Map::Map(unsigned int size)
    : m_size(size),
    m_grid(size, std::vector<int>(size, 1)),
    m_minRadius(100),
    selectedLine(nullptr),
    selectedTrain(nullptr) {}

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
    DEBUG_DEBUG("Adding city to line " + selectedLine->name + "...");
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
    DEBUG_DEBUG("Added city with name " + firstCity->name + " to line with name " + selectedLine->name);
}

void Map::SelectLine(Line* line) {
    DEBUG_DEBUG("Line of name " + line->name + " has been selected.");
    selectedLine = line;
}

void Map::SelectLine(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 5.0f; // Adjust this value to change click sensitivity

    for (auto& line : m_lines) {
        std::vector<City*> cities = line.GetCities();

        // Need at least 2 cities to form a line
        for (size_t i = 0; i < cities.size() - 1; ++i) {
            sf::Vector2f start = cities[i]->position;
            sf::Vector2f end = cities[i + 1]->position;

            // Calculate distance from point to line segment
            sf::Vector2f lineVec = end - start;
            sf::Vector2f pointVec = pos - start;

            float lineLength = std::sqrt(lineVec.x * lineVec.x + lineVec.y * lineVec.y);

            // Normalize line vector
            sf::Vector2f lineDir = lineVec / lineLength;

            // Project point onto line
            float projection = pointVec.x * lineDir.x + pointVec.y * lineDir.y;

            // Check if projection is within line segment
            if (projection >= 0 && projection <= lineLength) {
                // Calculate perpendicular distance
                sf::Vector2f projectedPoint = start + lineDir * projection;
                sf::Vector2f diff = pos - projectedPoint;
                float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);

                if (distance <= CLICK_THRESHOLD) {
                    SelectLine(&line);
                    DEBUG_DEBUG("Line selected!");
                    return;
                }
            }
        }
    }
}

void Map::DeselectLine() {
    DEBUG_DEBUG("Any line has been deselected.");
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

void Map::SelectTrain(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 10.0f; // Adjust this value as needed
    Train* closestTrain = nullptr;
    float closestDistanceSquared = CLICK_THRESHOLD * CLICK_THRESHOLD;

    for (auto& train : m_trains) {
        sf::Vector2f trainPos = train.GetPosition(); // Assuming Train has GetPosition()
        sf::Vector2f diff = trainPos - pos;
        float distanceSquared = diff.x * diff.x + diff.y * diff.y;

        if (distanceSquared < closestDistanceSquared) {
            closestDistanceSquared = distanceSquared;
            closestTrain = &train;
        }
    }

    if (closestTrain != nullptr) {
        SelectTrain(closestTrain);
    }
    else {
        DEBUG_DEBUG("No train found near the clicked position.");
        DeselectTrain();
    }
}

void Map::DeselectTrain() {
    DEBUG_DEBUG("Any train has been deselected.");
    for (auto& train : m_trains) {
        train.SetSelected(false);
    }
    selectedTrain = nullptr;
}

void Map::SelectTrain(Train* train) {
    if (selectedTrain != nullptr) {
        // Optionally, deselect the previously selected train
        selectedTrain->SetSelected(false); // Assuming Train has SetSelected
    }

    selectedTrain = train;

    if (selectedTrain != nullptr) {
        selectedTrain->SetSelected(true); // Highlight the selected train
        DEBUG_DEBUG("Train selected.");
    }
}

// Helper functions
void Map::Resize(unsigned int newSize) {
    m_grid.resize(newSize, std::vector<int>(newSize, 1));
    for (auto& row : m_grid) {
        row.resize(newSize, 1);
    }
    m_size = newSize;
}
