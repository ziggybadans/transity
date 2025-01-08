#include "Map.h"
#include "../Debug.h"
#include <queue>

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

        // Retrieve the index of the selected handle (node)
        int selectedHandleIndex = selectedLine->GetSelectedHandleIndex();

        if (selectedHandleIndex != -1 &&
            selectedHandleIndex != 0 &&
            selectedHandleIndex != selectedLine->GetPointCount() - 1) {
            CreateBranch(selectedLine, selectedHandleIndex, pos);
            return;
        }

        // Attempt to find if a city was clicked
        City* clickedCity = FindCityAtPosition(pos);

        if (clickedCity) {
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
            }
            else {
                // No specific handle is selected; default action is to add to the end
                AddToLineEnd(clickedCity->GetPosition());
            }
        }
        else {
            Node* genericNode = FindGenericNodeAtPosition(pos);
            if (genericNode) {
                // Retrieve the currently selected line
                Line* line = selectionManager.GetSelectedLine();
                if (!line) {
                    DEBUG_DEBUG("No line selected.");
                    return;
                }

                // Determine the starting point of the new segment: the last point on the line
                sf::Vector2f currentEnd = line->GetEndPosition();
                sf::Vector2f newNodePos = genericNode->GetPosition();

                // Check for parallel conflict with existing train lines
                if (WouldCauseParallelConflict(currentEnd, newNodePos)) {
                    DEBUG_DEBUG("Cannot add node. New segment would run parallel to an existing line with active trains.");
                    return;
                }

                // If no conflict, add the node to the line
                line->AddNode(genericNode);
                UpdateSharedSegments();
            }

            else {
                DEBUG_DEBUG("No valid city or generic node found at the clicked position.");
            }
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
    selectionManager.SelectLine(newLine);

    UpdateSharedSegments();

    DEBUG_DEBUG("New line created originating from " + firstCity->GetName() + " with name " + name + ". Selected line has been updated for new line.");
}

void Map::AddToLineStart(sf::Vector2f pos) {
    DEBUG_DEBUG("Adding city to the start of line " + selectionManager.GetSelectedLine()->GetName() + "...");

    City* newCity = FindCityAtPosition(pos);
    if (newCity == nullptr) {
        DEBUG_DEBUG("No valid city found at the clicked position to add to the line.");
        return;
    }

    // Check for parallel conflict before proceeding
    Line* selectedLine = selectionManager.GetSelectedLine();
    if (!selectedLine) {
        DEBUG_DEBUG("No line selected.");
        return;
    }

    sf::Vector2f currentStart = selectedLine->GetStartPosition();
    if (WouldCauseParallelConflict(newCity->GetPosition(), currentStart)) {
        DEBUG_DEBUG("Cannot add city. New segment would run parallel to an existing line with active trains.");
        return;
    }

    // Check if the city is already part of the line
    auto cityList = selectionManager.GetSelectedLine()->GetCities();
    if (std::find(cityList.begin(), cityList.end(), newCity) != cityList.end()) {
        DEBUG_DEBUG("The city is already part of the line.");
        return;
    }

    selectionManager.GetSelectedLine()->AddCityToStart(newCity);
    UpdateSharedSegments();
    DEBUG_DEBUG("Added city with name " + newCity->GetName() + " to the start of line " + selectionManager.GetSelectedLine()->GetName());
}

void Map::AddToLineEnd(sf::Vector2f pos) {
    DEBUG_DEBUG("Adding city to the end of line " + selectionManager.GetSelectedLine()->GetName() + "...");

    City* newCity = FindCityAtPosition(pos);
    if (newCity == nullptr) {
        DEBUG_DEBUG("No valid city found at the clicked position to add to the line.");
        return;
    }

    // Check for parallel conflict before proceeding
    Line* selectedLine = selectionManager.GetSelectedLine();
    if (!selectedLine) {
        DEBUG_DEBUG("No line selected.");
        return;
    }

    sf::Vector2f currentEnd = selectedLine->GetEndPosition();
    if (WouldCauseParallelConflict(currentEnd, newCity->GetPosition())) {
        DEBUG_DEBUG("Cannot add city. New segment would run parallel to an existing line with active trains.");
        return;
    }

    // Check if the city is already part of the line
    auto cityList = selectionManager.GetSelectedLine()->GetCities();
    if (std::find(cityList.begin(), cityList.end(), newCity) != cityList.end()) {
        DEBUG_DEBUG("The city is already part of the line.");
        return;
    }

    selectionManager.GetSelectedLine()->AddCityToEnd(newCity);
    UpdateSharedSegments();
    DEBUG_DEBUG("Added city with name " + newCity->GetName() + " to the end of line " + selectionManager.GetSelectedLine()->GetName());
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

void Map::AddTrain() {
    // Removed check for a selected line since multi-line functionality no longer requires it.
    if (startCityForTrain == nullptr || endCityForTrain == nullptr) {
        DEBUG_ERROR("AddTrain: Start or end city not selected.");
        return;
    }

    std::vector<Node*> routeNodes = FindRouteBetweenNodes(startCityForTrain, endCityForTrain);
    if (routeNodes.empty()) {
        DEBUG_ERROR("AddTrain: No route found between selected cities.");
        return;
    }

    std::vector<sf::Vector2f> fullPathPoints;
    Line* firstLine = nullptr;
    std::vector<sf::Vector2f> allStations;
    allStations.reserve(64);

    // Compute the multi-line path and station list
    for (size_t i = 0; i + 1 < routeNodes.size(); ++i) {
        Node* nodeA = routeNodes[i];
        Node* nodeB = routeNodes[i + 1];
        Line* connectingLine = nullptr;

        for (auto& line : m_lines) {
            const auto& points = line.GetPoints();
            for (size_t j = 0; j + 1 < points.size(); ++j) {
                if ((points[j].node == nodeA && points[j + 1].node == nodeB) ||
                    (points[j].node == nodeB && points[j + 1].node == nodeA)) {
                    connectingLine = &line;
                    break;
                }
            }
            if (connectingLine) break;
        }

        if (!connectingLine) {
            DEBUG_ERROR("AddTrain: No connecting line found between nodes.");
            return;
        }

        const auto& linePoints = connectingLine->GetPathPoints();
        const auto& cityIndices = connectingLine->GetCityIndices();
        int idxA = -1, idxB = -1;
        for (size_t j = 0; j < linePoints.size(); ++j) {
            if (ArePositionsEqual(linePoints[j], nodeA->GetPosition())) idxA = (int)j;
            if (ArePositionsEqual(linePoints[j], nodeB->GetPosition())) idxB = (int)j;
        }
        if (idxA == -1 || idxB == -1) {
            DEBUG_ERROR("AddTrain: Could not locate segment in connecting line.");
            return;
        }

        int startIdx = std::min(idxA, idxB);
        int endIdx = std::max(idxA, idxB);
        for (int k = startIdx; k <= endIdx; ++k) {
            fullPathPoints.push_back(linePoints[k]);
        }

        for (int cIdx : cityIndices) {
            sf::Vector2f cPos = connectingLine->GetPointPosition(cIdx);
            bool alreadyAdded = false;
            for (auto& st : allStations) {
                float dist = std::hypot(st.x - cPos.x, st.y - cPos.y);
                if (dist < 0.1f) {
                    alreadyAdded = true;
                    break;
                }
            }
            if (!alreadyAdded) {
                allStations.push_back(cPos);
            }
        }

        if (i == 0) {
            firstLine = connectingLine;
        }
    }

    if (fullPathPoints.empty()) {
        DEBUG_ERROR("AddTrain: Constructed path is empty.");
        return;
    }

    // Remove consecutive duplicate points from fullPathPoints
    std::vector<sf::Vector2f> filteredPathPoints;
    if (!fullPathPoints.empty()) {
        filteredPathPoints.push_back(fullPathPoints.front());
        for (size_t i = 1; i < fullPathPoints.size(); ++i) {
            if (!ArePositionsEqual(fullPathPoints[i], fullPathPoints[i - 1])) {
                filteredPathPoints.push_back(fullPathPoints[i]);
            }
        }
    }
    fullPathPoints = filteredPathPoints;

    std::string trainID = "Train" + std::to_string(m_trains.size() + 1);
    std::unique_ptr<Train> newTrain = std::make_unique<Train>(
        firstLine,
        trainID,
        fullPathPoints,
        allStations
    );

    if (firstLine) {
        firstLine->AddTrain(newTrain.get());
    }
    m_trains.push_back(std::move(newTrain));

    startCityForTrain = nullptr;
    endCityForTrain = nullptr;

    DEBUG_DEBUG("Added " + trainID + " with multi-line route. Station list size: " + std::to_string(allStations.size()));
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

void Map::RemoveCity() {
    City* selectedCity = selectionManager.GetSelectedCity();
    if (!selectedCity) {
        DEBUG_DEBUG("No city selected.");
        return;
    }

    // Check if the selected city is part of any line
    for (auto& line : m_lines) {
        auto lineCities = line.GetCities();
        if (std::find(lineCities.begin(), lineCities.end(), selectedCity) != lineCities.end()) {
            DEBUG_DEBUG("City " + selectedCity->GetName() + " has lines running through it; cannot delete.");
            return;
        }
    }

    // Capture city name before deletion for logging purposes
    std::string cityName = selectedCity->GetName();

    // Clear selection before removing city to avoid dangling pointer issues
    selectionManager.DeselectAll();

    // Remove the city from the list
    m_cities.remove_if([selectedCity](const City& city) {
        return &city == selectedCity;
        });

    DEBUG_DEBUG("City " + cityName + " removed.");
}

void Map::MoveCity(sf::Vector2f newPos) {
    City* selectedCity = selectionManager.GetSelectedCity();
    if (!selectedCity) {
        DEBUG_DEBUG("No city selected.");
        return;
    }

    // Check if the city is part of any line
    for (auto& line : m_lines) {
        auto lineCities = line.GetCities();
        if (std::find(lineCities.begin(), lineCities.end(), selectedCity) != lineCities.end()) {
            DEBUG_DEBUG("City " + selectedCity->GetName() + " has lines running through it; cannot move.");
            return;
        }
    }

    // Update the city's position
    selectedCity->SetPosition(newPos);
    DEBUG_DEBUG("City " + selectedCity->GetName() + " moved to new position.");
}

void Map::AddGenericNode(sf::Vector2f pos) {
    // Create a generic node with a unique name
    static int nodeSuffix = 1;
    std::string name = "Node" + std::to_string(nodeSuffix++);
    float radius = 5.0f;  // or any default radius for generic nodes
    m_nodes.emplace_back(name, pos, radius);
}

void Map::RemoveNode() {
    Node* selectedNode = selectionManager.GetSelectedNode();
    if (!selectedNode) {
        DEBUG_DEBUG("No node selected.");
        return;
    }
    // Remove from list
    m_nodes.remove_if([selectedNode](const Node& node) {
        return &node == selectedNode;
        });
    selectionManager.DeselectAll();
    DEBUG_DEBUG("Generic node removed.");
}

void Map::MoveNode(sf::Vector2f& newPos) {
    Node* selectedNode = selectionManager.GetSelectedNode();
    if (!selectedNode) {
        DEBUG_DEBUG("No node selected.");
        return;
    }
    selectedNode->SetPosition(newPos);
    DEBUG_DEBUG("Generic node moved to new position.");
}

Node* Map::FindGenericNodeAtPosition(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 10.0f;
    for (auto& node : m_nodes) {
        sf::Vector2f diff = node.GetPosition() - pos;
        if ((diff.x * diff.x + diff.y * diff.y) <= (CLICK_THRESHOLD * CLICK_THRESHOLD)) {
            return &node;
        }
    }
    return nullptr;
}

bool Map::WouldCauseParallelConflict(const sf::Vector2f& segStart, const sf::Vector2f& segEnd) {
    // Tolerance values: adjust angleTolerance and distanceThreshold as needed.
    const float angleTolerance = std::cos(5 * 3.14159265f / 180.0f); // 5 degrees tolerance
    const float distanceThreshold = 10.0f; // distance threshold in pixels

    for (auto& line : m_lines) {
        if (!line.HasTrains()) continue;  // Only consider lines with active trains

        auto points = line.GetPathPoints();
        for (size_t i = 0; i < points.size() - 1; ++i) {
            sf::Vector2f a = segStart;
            sf::Vector2f b = segEnd;
            sf::Vector2f c = points[i];
            sf::Vector2f d = points[i + 1];

            // Calculate direction vectors for both segments
            sf::Vector2f v1 = b - a;
            sf::Vector2f v2 = d - c;
            float len1 = std::sqrt(v1.x * v1.x + v1.y * v1.y);
            float len2 = std::sqrt(v2.x * v2.x + v2.y * v2.y);
            if (len1 == 0 || len2 == 0) continue;

            // Normalize direction vectors
            sf::Vector2f norm1 = v1 / len1;
            sf::Vector2f norm2 = v2 / len2;
            float dotVal = norm1.x * norm2.x + norm1.y * norm2.y;

            // Check if directions are nearly parallel
            if (std::abs(dotVal) < angleTolerance) continue;

            // Use distance calculations from endpoints to segments to assess proximity
            float distA = DistancePointToSegment(a, c, d);
            float distB = DistancePointToSegment(b, c, d);
            float distC = DistancePointToSegment(c, a, b);
            float distD = DistancePointToSegment(d, a, b);
            float minDist = std::min({ distA, distB, distC, distD });

            // If segments are close enough and nearly parallel, conflict detected
            if (minDist < distanceThreshold) return true;
        }
    }
    return false;
}

void Map::CreateBranch(Line* parentLine, int branchHandleIndex, sf::Vector2f pos) {
    if (!parentLine) return;

    Node* branchStart = parentLine->GetNodeAt(branchHandleIndex);
    if (!branchStart) {
        DEBUG_DEBUG("CreateBranch: Invalid branch start node.");
        return;
    }

    static int lineSuffix = 1;
    std::string name = "Line" + std::to_string(lineSuffix++);

    // Create a new branch line starting from the branch point
    m_lines.emplace_back(branchStart, name);
    Line* newLine = &m_lines.back();
    selectionManager.SelectLine(newLine);

    // Extend the new branch line based on the clicked position
    City* clickedCity = FindCityAtPosition(pos);
    if (clickedCity) {
        AddToLineEnd(clickedCity->GetPosition());
    }
    else {
        Node* genericNode = FindGenericNodeAtPosition(pos);
        if (genericNode) {
            Line* selectedLine = selectionManager.GetSelectedLine();
            if (!selectedLine) {
                DEBUG_DEBUG("CreateBranch: No selected line after branch creation.");
                return;
            }
            sf::Vector2f currentEnd = selectedLine->GetEndPosition();
            sf::Vector2f newNodePos = genericNode->GetPosition();
            if (WouldCauseParallelConflict(currentEnd, newNodePos)) {
                DEBUG_DEBUG("Cannot add node to branch. New segment would run parallel to an existing line with active trains.");
                return;
            }
            selectedLine->AddNode(genericNode);
            UpdateSharedSegments();
        }
        else {
            DEBUG_DEBUG("No valid city or generic node found at the clicked position for branch extension.");
        }
    }

    UpdateSharedSegments();
}

std::vector<Node*> Map::FindRouteBetweenNodes(Node* start, Node* end) {
    if (!start || !end) return {};

    // Build graph: node -> adjacent nodes
    std::unordered_map<Node*, std::vector<Node*>> graph;
    for (auto& line : m_lines) {
        const auto& points = line.GetPoints(); // Get all points (cities/nodes) on the line
        for (size_t i = 0; i + 1 < points.size(); ++i) {
            Node* A = points[i].node;
            Node* B = points[i + 1].node;
            graph[A].push_back(B);
            graph[B].push_back(A);
        }
    }

    // BFS setup
    std::queue<Node*> frontier;
    std::unordered_map<Node*, Node*> cameFrom;
    frontier.push(start);
    cameFrom[start] = nullptr;

    while (!frontier.empty()) {
        Node* current = frontier.front();
        frontier.pop();

        if (current == end) break;

        for (Node* next : graph[current]) {
            if (cameFrom.find(next) == cameFrom.end()) {
                frontier.push(next);
                cameFrom[next] = current;
            }
        }
    }

    // If end wasn't reached, return empty
    if (cameFrom.find(end) == cameFrom.end()) {
        return {};
    }

    // Reconstruct path from start to end
    std::vector<Node*> path;
    for (Node* at = end; at != nullptr; at = cameFrom[at]) {
        path.push_back(at);
    }
    std::reverse(path.begin(), path.end());
    return path;
}