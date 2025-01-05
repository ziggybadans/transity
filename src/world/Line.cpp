// Line.cpp
#include "Line.h"
#include "../entity/Train.h" // Ensure that Train.h is included if needed
#include "../Debug.h" // Assuming Debug.h provides debugging utilities
#include <cmath>

// Constructor
Line::Line(City* startCity, const std::string& lineName,
    const sf::Color& lineColor, float lineThickness)
    : name(lineName), color(lineColor), thickness(lineThickness), selected(false) {

    // Initialize with the starting city
    LinePoint p(true, startCity->position, startCity);
    points.push_back(p);

    // Initialize the first handle
    handles.emplace_back(0, false);
}

// Destructor
Line::~Line() {
    // If ownership of Trains is managed elsewhere, no need to delete them here
    // Otherwise, iterate and delete if necessary
    // Currently, assuming Line does not own the Train objects
}

// Adds a city to the start of the line
void Line::AddCityToStart(City* city) {
    LinePoint p(true, city->position, city);
    points.insert(points.begin(), p);

    // Insert a new handle at the beginning
    handles.insert(handles.begin(), Handle(0, false));

    // Update existing handle indices
    for (size_t i = 1; i < handles.size(); ++i) {
        handles[i].index = static_cast<int>(i);
    }
}

// Adds a city to the end of the line
void Line::AddCityToEnd(City* city) {
    LinePoint p(true, city->position, city);
    points.push_back(p);

    // Add a new handle at the end
    handles.emplace_back(static_cast<int>(handles.size()), false);
}

// Inserts a city after a specified index
void Line::InsertCityAfter(int index, City* city) {
    if (index < 0 || index >= static_cast<int>(points.size())) {
        DEBUG_ERROR("InsertCityAfter: Invalid handle index.");
        return;
    }

    // Create a new LinePoint for the city
    LinePoint newPoint(true, city->position, city);

    // Insert the new point after the specified index
    points.insert(points.begin() + index + 1, newPoint);

    // Insert a new handle for this point
    handles.emplace(handles.begin() + index + 1, Handle(index + 1, false));

    // Update handle indices for subsequent handles
    for (size_t i = index + 2; i < handles.size(); ++i) {
        handles[i].index = static_cast<int>(i);
    }
}

// Adds a non-city node to the line
void Line::AddNode(sf::Vector2f pos) {
    LinePoint p(false, pos, nullptr);
    points.push_back(p);

    // Add a new handle for the node
    handles.emplace_back(static_cast<int>(handles.size()), false);
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

    if (points.empty()) {
        return path;
    }

    // Return the list of points as the path
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
    DeselectHandles();
    for (auto& handle : handles) {
        if (handle.index == index) {
            handle.isSelected = true;
            break;
        }
    }
}

// Deselects all handles
void Line::DeselectHandles() {
    for (auto& handle : handles) {
        handle.isSelected = false;
    }
}

// Retrieves the index of the currently selected handle
int Line::GetSelectedHandleIndex() const {
    for (const auto& handle : handles) {
        if (handle.isSelected) {
            return handle.index;
        }
    }
    return -1; // No handle selected
}

// Moves a handle to a new position
void Line::MoveHandle(int index, sf::Vector2f newPos) {
    if (index < 0 || index >= static_cast<int>(points.size())) {
        DEBUG_ERROR("MoveHandle: Invalid handle index.");
        return;
    }

    // If this point is a city, move the city as well
    if (points[index].isCity && points[index].city != nullptr) {
        points[index].city->position = newPos;
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
