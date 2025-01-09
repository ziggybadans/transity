#include "Line.h"
#include "../entity/Train.h" // Ensure that Train.h is included if needed
#include "../Debug.h" // Assuming Debug.h provides debugging utilities
#include "City.h"
#include "Node.h"
#include <cmath>

// Constructor
Line::Line(City* startCity, const std::string& lineName,
    const sf::Color& lineColor, float lineThickness)
    : name(lineName), color(lineColor), thickness(lineThickness), selected(false) 
{
    // Initialize with the starting city as a Node
    points.push_back(LinePoint(startCity));
    handleManager.AddHandle(0);
}

Line::Line(Node* startNode, const std::string& lineName,
    const sf::Color& lineColor, float lineThickness)
    : name(lineName), color(lineColor), thickness(lineThickness), selected(false)
{
    points.push_back(LinePoint(startNode));
    handleManager.AddHandle(0);
}

// Adds a city to the start of the line
void Line::AddCityToStart(City* city)
{
    if (!city) {
        DEBUG_ERROR("AddCityToStart: nullptr city provided.");
        return;
    }
    points.insert(points.begin(), LinePoint(city));
    handleManager.InsertHandle(0, 0);
}

// Adds a city to the end of the line
void Line::AddCityToEnd(City* city)
{
    if (!city) {
        DEBUG_ERROR("AddCityToEnd: nullptr city provided.");
        return;
    }
    points.emplace_back(city);
    handleManager.AddHandle(static_cast<int>(points.size() - 1));
}

// Inserts a city after a specified index
void Line::InsertCityAfter(int index, City* city)
{
    if (!city) {
        DEBUG_ERROR("InsertCityAfter: nullptr city provided.");
        return;
    }
    if (index < 0 || index >= static_cast<int>(points.size())) {
        DEBUG_ERROR("InsertCityAfter: Invalid handle index.");
        return;
    }

    points.emplace(points.begin() + index + 1, LinePoint(city));
    handleManager.InsertHandle(index + 1, index + 1);
}

// Adds a non-city node to the line
void Line::AddNode(Node* node) {
    points.emplace_back(LinePoint(node));
    handleManager.AddHandle(static_cast<int>(points.size() - 1));
}

void Line::CalculateOffsets(const std::vector<Segment>& sharedSegments) {
    offsetInfos.clear();
    offsetInfos.reserve(points.size() - 1); // One OffsetInfo per segment

    for (size_t i = 0; i < points.size() - 1; ++i) {
        // Check if this segment is shared by finding a matching Segment in sharedSegments
        auto it = std::find_if(sharedSegments.begin(), sharedSegments.end(),
            [this, i](const Segment& seg) {
                return seg.startPointIndex == static_cast<int>(i)
                    && seg.endPointIndex == static_cast<int>(i + 1)
                    && std::find(seg.overlappingLines.begin(), seg.overlappingLines.end(), this) != seg.overlappingLines.end();
            });

        if (it != sharedSegments.end() && it->overlappingLines.size() > 1) {
            // This segment is shared by multiple lines
            // Determine this line's index in overlappingLines
            auto lineIt = std::find(it->overlappingLines.begin(), it->overlappingLines.end(), this);
            if (lineIt != it->overlappingLines.end()) {
                int lineIndex = static_cast<int>(std::distance(it->overlappingLines.begin(), lineIt));

                // Define offset step based on line thickness
                float offsetStep = thickness + 2.0f; // Thickness plus padding
                float totalLines = static_cast<float>(it->overlappingLines.size());
                float halfTotal = (totalLines - 1.0f) / 2.0f;

                // Calculate offset magnitude centered around zero
                float offsetMagnitude = (static_cast<float>(lineIndex) - halfTotal) * offsetStep;

                // Calculate the perpendicular vector
                sf::Vector2f direction = points[i + 1].node->GetPosition() - points[i].node->GetPosition();
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length == 0) {
                    offsetInfos.emplace_back(OffsetInfo{ sf::Vector2f(0.f, 0.f), 0.f });
                    continue;
                }
                sf::Vector2f unitDir = direction / length;
                sf::Vector2f perpendicular(-unitDir.y, unitDir.x);

                sf::Vector2f offsetVec = perpendicular * offsetMagnitude;

                // Define transition length (e.g., 20 pixels)
                float transitionLen = 20.0f;

                offsetInfos.emplace_back(OffsetInfo{ offsetVec, transitionLen });
            }
            else {
                // Should not happen
                offsetInfos.emplace_back(OffsetInfo{ sf::Vector2f(0.f, 0.f), 0.f });
            }
        }
        else {
            // Not a shared segment
            offsetInfos.emplace_back(OffsetInfo{ sf::Vector2f(0.f, 0.f), 0.f });
        }
    }
}

std::vector<sf::Vector2f> Line::GetAdjustedPathPoints() const {
    std::vector<sf::Vector2f> adjustedPoints;
    adjustedPoints.reserve(points.size());

    for (size_t i = 0; i < points.size(); ++i) {
        if (i == 0 || i == points.size() - 1) {
            // Start and end points are always unoffset
            adjustedPoints.emplace_back(points[i].node->GetPosition());
            continue;
        }

        // Average offsets from previous and next segments for smoothness
        sf::Vector2f offsetPrev = (i - 1 < offsetInfos.size()) ? offsetInfos[i - 1].offsetVector : sf::Vector2f(0.f, 0.f);
        sf::Vector2f offsetNext = (i < offsetInfos.size()) ? offsetInfos[i].offsetVector : sf::Vector2f(0.f, 0.f);

        // Average the two offsets
        sf::Vector2f averageOffset = (offsetPrev + offsetNext) / 2.0f;

        // Apply the average offset
        adjustedPoints.emplace_back(points[i].node->GetPosition() + averageOffset);
    }

    // Adjust the start and end points with the first and last segments' offsets
    if (!points.empty() && !adjustedPoints.empty()) {
        adjustedPoints[0] = points[0].node->GetPosition() + ((offsetInfos.empty()) ? sf::Vector2f(0.f, 0.f) : offsetInfos[0].offsetVector);
        adjustedPoints.back() = points.back().node->GetPosition() + ((offsetInfos.empty()) ? sf::Vector2f(0.f, 0.f) : offsetInfos.back().offsetVector);
    }

    return adjustedPoints;
}

// Retrieves all cities on the line
std::vector<City*> Line::GetCities() const {
    std::vector<City*> cityList;
    for (const auto& point : points) {
        if (City* city = dynamic_cast<City*>(point.node)) {
            cityList.push_back(city);
        }
    }
    return cityList;
}

// Retrieves all path points (positions) on the line
std::vector<sf::Vector2f> Line::GetPathPoints() const {
    std::vector<sf::Vector2f> path;
    path.reserve(points.size());

    for (const auto& point : points) {
        path.emplace_back(point.node->GetPosition());
    }

    return path;
}

// Retrieves the indices of all cities within the points vector
std::vector<int> Line::GetCityIndices() const
{
    std::vector<int> cityIndices;
    cityIndices.reserve(points.size());
    for (size_t i = 0; i < points.size(); ++i) {
        if (City* city = dynamic_cast<City*>(points[i].node)) {
            cityIndices.emplace_back(static_cast<int>(i));
        }
    }
    return cityIndices;
}

// Retrieves the starting position of the line
sf::Vector2f Line::GetStartPosition() const {
    if (points.empty()) return sf::Vector2f(0.f, 0.f);
    return points.front().node->GetPosition();
}

// Retrieves the ending position of the line
sf::Vector2f Line::GetEndPosition() const {
    if (points.empty()) return sf::Vector2f(0.f, 0.f);
    return points.back().node->GetPosition();
}

// Retrieves the position of a specific point by index
sf::Vector2f Line::GetPointPosition(int index) const {
    if (index >= 0 && index < static_cast<int>(points.size())) {
        return points[index].node->GetPosition();
    }
    DEBUG_ERROR("GetPointPosition: Index out of range.");
    return sf::Vector2f(0.f, 0.f);
}

// Selects a handle by index
void Line::SelectHandle(int index) {
    handleManager.SelectHandle(index);
}

// Deselects all handles
void Line::DeselectHandles() {
    handleManager.DeselectAll();
}

// Retrieves the index of the currently selected handle
int Line::GetSelectedHandleIndex() const {
    return handleManager.GetSelectedHandleIndex();
}

// Moves a handle to a new position
void Line::MoveHandle(int index, sf::Vector2f newPos) {
    if (index < 0 || index >= static_cast<int>(points.size())) {
        DEBUG_ERROR("MoveHandle: Invalid handle index.");
        return;
    }

    // Update the node’s position using SetPosition, regardless of its type
    points[index].node->SetPosition(newPos);
}

// Adds a train to the line
void Line::AddTrain(Train* train) {
    if (train != nullptr) {
        trains.emplace_back(train);
    }
}

// Removes a train from the line
void Line::RemoveTrain(Train* train) {
    trains.erase(std::remove(trains.begin(), trains.end(), train), trains.end());
}

// Utility function to normalize a vector
sf::Vector2f Line::Normalize(const sf::Vector2f& vec) const {
    float length = std::sqrt(vec.x * vec.x + vec.y * vec.y);
    if (length != 0)
        return vec / length;
    else
        return sf::Vector2f(0.f, 0.f);
}

sf::Vector2f Line::GetPerpendicularVector(int segmentIndex) const {
    if (segmentIndex < 0 || segmentIndex >= static_cast<int>(points.size() - 1))
        return sf::Vector2f(0.f, 0.f);

    sf::Vector2f direction = points[segmentIndex + 1].node->GetPosition() - points[segmentIndex].node->GetPosition();
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    if (length == 0)
        return sf::Vector2f(0.f, 0.f);

    sf::Vector2f unitDir = direction / length;
    sf::Vector2f perpendicular(-unitDir.y, unitDir.x);
    return perpendicular;
}

std::vector<int> Line::GetIndicesBetweenCities(City* cityA, City* cityB) const
{
    // 1. Find the indices of cityA and cityB in the 'points' array
    int indexA = -1;
    int indexB = -1;

    for (int i = 0; i < static_cast<int>(points.size()); i++) {
        if (City* city = dynamic_cast<City*>(points[i].node)) {
            if (points[i].node == cityA) {
                indexA = i;
            }
        }
        if (City* city = dynamic_cast<City*>(points[i].node)) {
            if (points[i].node == cityB) {
                indexB = i;
            }
        }
    }

    // If either city was not found, return empty
    if (indexA == -1 || indexB == -1) {
        return {};
    }

    // 2. Ensure indexA < indexB for forward slicing
    bool reverseOrder = false;
    if (indexA > indexB) {
        std::swap(indexA, indexB);
        reverseOrder = true;
    }

    // 3. Gather all indices from indexA to indexB (inclusive)
    std::vector<int> routeSegment;
    routeSegment.reserve(indexB - indexA + 1);

    for (int i = indexA; i <= indexB; i++) {
        routeSegment.push_back(i);
    }

    // 4. If the original order was reversed, we reverse the collected segment
    //    so the path goes from cityA -> cityB in correct order
    if (reverseOrder) {
        std::reverse(routeSegment.begin(), routeSegment.end());
    }

    return routeSegment;
}

Node* Line::GetNodeAt(int index) const {
    if (index >= 0 && index < static_cast<int>(points.size()))
        return points[index].node;
    return nullptr;
}

int Line::GetPointCount() const {
    return static_cast<int>(points.size());
}