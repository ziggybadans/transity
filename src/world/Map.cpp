#include "Map.h"
#include "../Debug.h"

void Map::SelectObject(const sf::Vector2f& pos) {
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
                    UpdateSharedSegments();
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
            UpdateSharedSegments();
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

    UpdateSharedSegments();

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
    UpdateSharedSegments();
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
    UpdateSharedSegments();
    DEBUG_DEBUG("Added city with name " + newCity->GetName() + " to the end of line " + GetSelectedLine()->GetName());
}

bool Map::SelectLine(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 5.0f; // Adjust as needed
    float bestDistance = CLICK_THRESHOLD;
    Line* closestLine = nullptr;

    for (auto& line : m_lines) {
        // Use the adjusted path points that account for offsets
        const auto& pathPoints = line.GetAdjustedPathPoints();

        // Iterate through all straight segments of the line
        for (size_t i = 0; i < pathPoints.size() - 1; ++i) {
            sf::Vector2f start = pathPoints[i];
            sf::Vector2f end = pathPoints[i + 1];

            // Calculate distance from click position to the segment
            float distance = DistancePointToSegment(pos, start, end);
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

void Map::UpdateSharedSegments() {
    // Temporary map to track segments and associated lines
    // Using sorted pair of positions as the key
    struct PositionPair {
        sf::Vector2f start;
        sf::Vector2f end;

        bool operator==(const PositionPair& other) const {
            return (ArePositionsEqual(start, other.start) && ArePositionsEqual(end, other.end)) ||
                (ArePositionsEqual(start, other.end) && ArePositionsEqual(end, other.start));
        }
    };

    struct PositionPairHash {
        std::size_t operator()(const PositionPair& pair) const {
            std::size_t h1 = std::hash<float>()(std::round(pair.start.x * 10.0f) / 10.0f);
            std::size_t h2 = std::hash<float>()(std::round(pair.start.y * 10.0f) / 10.0f);
            std::size_t h3 = std::hash<float>()(std::round(pair.end.x * 10.0f) / 10.0f);
            std::size_t h4 = std::hash<float>()(std::round(pair.end.y * 10.0f) / 10.0f);
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
        }
    };

    std::unordered_map<PositionPair, std::vector<Line*>, PositionPairHash> segmentMap;

    // Iterate through all lines and their segments
    for (auto& line : m_lines) {
        const auto& pathPoints = line.GetPathPoints();
        for (size_t i = 0; i < pathPoints.size() - 1; ++i) {
            sf::Vector2f start = pathPoints[i];
            sf::Vector2f end = pathPoints[i + 1];

            // Create a sorted PositionPair to ensure consistency
            PositionPair pair;
            if (start.x < end.x || (start.x == end.x && start.y <= end.y)) {
                pair.start = start;
                pair.end = end;
            }
            else {
                pair.start = end;
                pair.end = start;
            }

            // Insert the line into the segment's line list
            segmentMap[pair].emplace_back(&line);
        }
    }

    // Clear existing shared segments
    sharedSegments.clear();

    // Populate sharedSegments with segments shared by multiple lines
    for (const auto& [pair, lines] : segmentMap) {
        if (lines.size() > 1) {
            // For each line that shares this segment, find the segment indices and create a Segment struct
            for (auto* line : lines) {
                const auto& pathPoints = line->GetPathPoints();
                for (size_t i = 0; i < pathPoints.size() - 1; ++i) {
                    if ((ArePositionsEqual(pathPoints[i], pair.start) && ArePositionsEqual(pathPoints[i + 1], pair.end)) ||
                        (ArePositionsEqual(pathPoints[i], pair.end) && ArePositionsEqual(pathPoints[i + 1], pair.start))) {
                        Segment seg(i, i + 1);
                        seg.overlappingLines = lines; // All lines sharing this segment
                        sharedSegments.emplace_back(seg);
                        break; // Move to next line
                    }
                }
            }
        }
    }

    // Now, for each line, calculate its offsets based on the shared segments
    for (auto& line : m_lines) {
        line.CalculateOffsets(sharedSegments);
    }
}

void Map::UseTrainPlaceMode(sf::Vector2f pos, bool left) {
    City* clickedCity = FindCityAtPosition(pos);
    if (clickedCity) {
        if (left) {
            startCityForTrain = clickedCity;
        }
        else {
            endCityForTrain = clickedCity;
        }
    }

    if (startCityForTrain != nullptr && endCityForTrain != nullptr) {
        stateManager.SetState("TrainPlaceVerified", true);
    }
}

void Map::AddTrain()
{
    if (!isLineSelected())
    {
        DEBUG_ERROR("AddTrain: No line selected.");
        return;
    }

    if (startCityForTrain == nullptr || endCityForTrain == nullptr)
    {
        DEBUG_ERROR("AddTrain: Start or end city not selected.");
        return;
    }

    Line* selectedLine = GetSelectedLine();

    // Check that both cities are on the selected line
    std::vector<City*> citiesOnLine = selectedLine->GetCities();
    if (std::find(citiesOnLine.begin(), citiesOnLine.end(), startCityForTrain) == citiesOnLine.end())
    {
        DEBUG_ERROR("AddTrain: Start city is not on the selected line.");
        return;
    }

    if (std::find(citiesOnLine.begin(), citiesOnLine.end(), endCityForTrain) == citiesOnLine.end())
    {
        DEBUG_ERROR("AddTrain: End city is not on the selected line.");
        return;
    }

    // Get indices between start and end cities
    std::vector<int> cityIndices = selectedLine->GetIndicesBetweenCities(startCityForTrain, endCityForTrain);

    if (cityIndices.empty())
    {
        DEBUG_ERROR("AddTrain: Invalid path between selected cities.");
        return;
    }

    // Use adjusted points for train path
    std::vector<sf::Vector2f> pathPoints;
    auto adjustedPoints = selectedLine->GetAdjustedPathPoints();
    for (int index : cityIndices)
    {
        pathPoints.push_back(adjustedPoints[index]);
    }

    // Create and add the new train
    std::string trainID = "Train" + std::to_string(m_trains.size() + 1);
    std::unique_ptr<Train> newTrain = std::make_unique<Train>(selectedLine, trainID, pathPoints);

    selectedLine->AddTrain(newTrain.get());
    m_trains.push_back(std::move(newTrain));

    startCityForTrain = nullptr;
    endCityForTrain = nullptr;

    DEBUG_DEBUG("Added " + trainID + " to line " + selectedLine->GetName());
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

// Normalize the segment by ordering the indices
std::pair<int, int> Map::NormalizeSegment(int startIndex, int endIndex) const {
    if (startIndex < endIndex)
        return { startIndex, endIndex };
    else
        return { endIndex, startIndex };
}

std::string Map::GenerateSegmentKey(const sf::Vector2f& start, const sf::Vector2f& end) const {
    // Define precision (e.g., 1 decimal place)
    auto roundPos = [](const sf::Vector2f& pos) -> sf::Vector2f {
        return sf::Vector2f(
            std::round(pos.x * 10.0f) / 10.0f,
            std::round(pos.y * 10.0f) / 10.0f
        );
        };

    sf::Vector2f roundedStart = roundPos(start);
    sf::Vector2f roundedEnd = roundPos(end);

    // Order the points to ensure consistency (A-B same as B-A)
    if (roundedStart.x < roundedEnd.x ||
        (roundedStart.x == roundedEnd.x && roundedStart.y <= roundedEnd.y)) {
        // Start comes first
    }
    else {
        // Swap start and end
        std::swap(roundedStart, roundedEnd);
    }

    // Create key string
    return std::to_string(roundedStart.x) + "," + std::to_string(roundedStart.y) + "-" +
        std::to_string(roundedEnd.x) + "," + std::to_string(roundedEnd.y);
}

bool Map::ArePositionsEqual(const sf::Vector2f& pos1, const sf::Vector2f& pos2, float epsilon) {
    return (std::abs(pos1.x - pos2.x) <= epsilon) && (std::abs(pos1.y - pos2.y) <= epsilon);
}