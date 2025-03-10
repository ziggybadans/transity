#include "Map.h"
#include "../entity/Passenger.h"
#include "../Debug.h"
#include <queue>

void Map::SetTile(unsigned int x, unsigned int y, int value) {
    if (x >= m_size || y >= m_size)
        throw std::out_of_range("Invalid tile coordinates");
    m_grid[x][y] = value;
}

City* Map::FindCityAtPosition(sf::Vector2f pos) {
    const float CLICK_THRESHOLD = 10.0f; // Adjust as needed

    for (auto& city : GetCities()) {
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

    for (auto& line : GetLines()) {
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

nlohmann::json Map::Serialize() {
    nlohmann::json j;

    // Serialize cities
    j["cities"] = nlohmann::json::array();
    for (const City& city : GetCities()) {
        j["cities"].push_back(city.Serialize());
    }

    // Serialize nodes
    j["nodes"] = nlohmann::json::array();
    for (const Node& node : m_nodes) {
        j["nodes"].push_back(node.Serialize());
    }

    j["lines"] = nlohmann::json::array();
    for (const Line& line : GetLines()) {
        j["lines"].push_back(line.Serialize());
    }

    // Serialize trains
    j["trains"] = nlohmann::json::array();
    for (const auto& trainPtr : trainManager.GetTrains()) {
        j["trains"].push_back(trainPtr->Serialize());
    }

    return j;
}

void Map::Deserialize(const nlohmann::json& j) {
    // Clear current cities from the manager
    auto& cities = cityManager.GetCities();
    cities.clear();

    // Deserialize cities
    if (j.contains("cities") && j["cities"].is_array()) {
        for (const auto& cityJson : j["cities"]) {
            City city("", sf::Vector2f(0.0f, 0.0f), 0);
            city.Deserialize(cityJson);
            cities.push_back(city);
        }
    }

    // Deserialize nodes
    m_nodes.clear();
    if (j.contains("nodes") && j["nodes"].is_array()) {
        for (const auto& nodeJson : j["nodes"]) {
            // Initialize with dummy/default values; will be overwritten by Deserialize
            Node node("", sf::Vector2f(0.0f, 0.0f));
            node.Deserialize(nodeJson);
            m_nodes.push_back(node);
        }
    }

    auto& lines = lineManager.GetLines();
    lines.clear();
    if (j.contains("lines") && j["lines"].is_array()) {
        for (const auto& lineJson : j["lines"]) {
            // Create a blank line with dummy name & color; 
            // it will be overridden by Deserialize
            Line line(static_cast<City*>(nullptr), "tmp");

            // 1) Just read the JSON into the line's member variables & unresolvedPoints
            line.Deserialize(lineJson);

            // 2) Now fix up each point by finding the actual city or node
            for (auto& up : line.unresolvedPoints) {
                if (up.type == "city") {
                    // find city in 'cities' by name
                    City* cityPtr = nullptr;
                    for (auto& c : cities) {
                        if (c.GetName() == up.name) {
                            cityPtr = &c;
                            break;
                        }
                    }
                    if (cityPtr) {
                        // Add it as a city point
                        line.AddCityToEnd(cityPtr);
                    }
                    else {
                        // handle error: city not found
                    }
                }
                else if (up.type == "node") {
                    // find node in 'm_nodes' by name
                    Node* nodePtr = nullptr;
                    for (auto& n : m_nodes) {
                        if (n.GetName() == up.name) {
                            nodePtr = &n;
                            break;
                        }
                    }
                    if (nodePtr) {
                        line.AddNode(nodePtr);
                    }
                    else {
                        // handle error: node not found
                    }
                }
            }

            // 3) Clear the unresolved points
            line.unresolvedPoints.clear();

            // 4) Finally add this line to the lines container
            lines.push_back(line);
        }
    }

    // After lines are deserialized:
    // Create a lookup for lines by name to resolve route pointers.
    std::unordered_map<std::string, Line*> linesByName;
    for (Line& line : lineManager.GetLines()) {
        linesByName[line.GetName()] = &line;
    }

    // Deserialize trains
    auto& trains = trainManager.GetTrains();
    for (auto& trainPtr : trains) {
        Line* route = trainPtr->GetRoute();
        if (route) {
            route->RemoveTrain(trainPtr.get());
            trainPtr->SetRoute(nullptr);  // Clear route to avoid double removal
        }
    }
    trains.clear();  // Now safe to clear without triggering route removal in destructors
    if (j.contains("trains") && j["trains"].is_array()) {
        for (const auto& trainJson : j["trains"]) {
            // Create a temporary Train with placeholder values.
            std::vector<sf::Vector2f> dummyPathPoints;
            std::vector<sf::Vector2f> dummyStationPositions;
            auto newTrain = std::make_unique<Train>(
                nullptr,               // Temporary route pointer
                "",                    // Temporary ID
                dummyPathPoints,
                dummyStationPositions
            );

            // Deserialize train data from JSON.
            newTrain->Deserialize(trainJson);

            // Resolve and set the route using the stored route name.
            std::string routeName = trainJson["route"].get<std::string>();
            if (linesByName.find(routeName) != linesByName.end()) {
                newTrain->SetRoute(linesByName[routeName]);
                linesByName[routeName]->AddTrain(newTrain.get());
            }
            else {
                // Handle missing route scenario if necessary
            }

            // Add train to the manager's list.
            trains.push_back(std::move(newTrain));
        }
    }

    // Build a lookup table for cities by name
    std::unordered_map<std::string, City*> cityLookup;
    for (City& city : GetCities()) {
        cityLookup[city.GetName()] = &city;
    }

    // Resolve passenger pointers for passengers onboard each train
    for (auto& trainPtr : trainManager.GetTrains()) {
        for (Passenger* p : trainPtr->GetPassengers()) {
            p->ResolvePointers(cityLookup);
            // If passenger state is Waiting, add to city's waiting list
            if (p->GetState() == PassengerState::Waiting && p->GetCurrentCity()) {
                p->GetCurrentCity()->AddWaitingPassenger(p);
            }
            // For passengers still OnTrain, additional integration can be done if needed
        }
    }
}