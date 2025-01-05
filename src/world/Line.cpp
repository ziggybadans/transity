#include "Line.h"
#include "../entity/Train.h" // Ensure that Train.h is included if needed
#include "../Debug.h" // Assuming Debug.h provides debugging utilities
#include <cmath>

// Constructor
Line::Line(City* startCity, const std::string& lineName,
    const sf::Color& lineColor, float lineThickness)
    : name(lineName), color(lineColor), thickness(lineThickness), selected(false) {

    // Initialize with the starting city
    LinePoint p(true, startCity->GetPosition(), startCity);
    points.push_back(p);

    // Initialize the handle manager with the first handle
    handleManager.AddHandle(0);
}

// Destructor
Line::~Line() {
    // Assuming Line does not own the Train objects
}

// Adds a city to the start of the line
void Line::AddCityToStart(City* city) {
    LinePoint p(true, city->GetPosition(), city);
    points.insert(points.begin(), p);
    handleManager.InsertHandle(0, 0);
}

// Adds a city to the end of the line
void Line::AddCityToEnd(City* city) {
    points.emplace_back(true, city->GetPosition(), city);
    handleManager.AddHandle(static_cast<int>(points.size() - 1));
}

// Inserts a city after a specified index
void Line::InsertCityAfter(int index, City* city) {
    if (index < 0 || index >= static_cast<int>(points.size())) {
        DEBUG_ERROR("InsertCityAfter: Invalid handle index.");
        return;
    }

    LinePoint newPoint(true, city->GetPosition(), city);
    points.insert(points.begin() + index + 1, newPoint);
    handleManager.InsertHandle(index + 1, index + 1);
}

// Adds a non-city node to the line
void Line::AddNode(sf::Vector2f pos) {
    LinePoint p(false, pos, nullptr);
    points.emplace_back(p);
    handleManager.AddHandle(static_cast<int>(points.size() - 1));
}

// Retrieves all cities on the line
const std::vector<City*> Line::GetCities() const {
    std::vector<City*> cityList;
    for (const auto& point : points) {
        if (point.isCity && point.city != nullptr) {
            cityList.push_back(point.city);
        }
    }
    return cityList;
}

// Retrieves all path points (positions) on the line
std::vector<sf::Vector2f> Line::GetPathPoints() const {
    std::vector<sf::Vector2f> path;
    path.reserve(points.size());

    for (const auto& point : points) {
        path.emplace_back(point.position);
    }

    return path;
}

// Retrieves the indices of all cities within the points vector
const std::vector<int> Line::GetCityIndices() const {
    std::vector<int> cityIndices;
    for (size_t i = 0; i < points.size(); ++i) {
        if (points[i].isCity) {
            cityIndices.emplace_back(static_cast<int>(i));
        }
    }
    return cityIndices;
}

// Retrieves the starting position of the line
sf::Vector2f Line::GetStartPosition() const {
    if (points.empty()) return sf::Vector2f(0.f, 0.f);
    return points.front().position;
}

// Retrieves the ending position of the line
sf::Vector2f Line::GetEndPosition() const {
    if (points.empty()) return sf::Vector2f(0.f, 0.f);
    return points.back().position;
}

// Retrieves the position of a specific point by index
sf::Vector2f Line::GetPointPosition(int index) const {
    if (index >= 0 && index < static_cast<int>(points.size())) {
        return points[index].position;
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

    // If this point is a city, move the city as well
    if (points[index].isCity && points[index].city != nullptr) {
        points[index].city->SetPosition(newPos);
    }

    // Always move the line point
    points[index].position = newPos;
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
