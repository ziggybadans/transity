#include "Line.h"
#include "../Debug.h"

void Line::AddCityToStart(City* city) {
    LinePoint p;
    p.isCity = true;
    p.position = city->position;
    p.city = city;
    points.insert(points.begin(), p);

    // Insert a new handle at the beginning
    handles.insert(handles.begin(), Handle(0));
    // Update existing handle indices
    for (size_t i = 1; i < handles.size(); ++i) {
        handles[i].index = i;
    }

    UpdateBezierSegments();
}

void Line::AddCityToEnd(City* city) {
    LinePoint p;
    p.isCity = true;
    p.position = city->position;
    p.city = city;
    points.push_back(p);

    // Add a new handle at the end
    handles.emplace_back(handles.size());

    UpdateBezierSegments();
}

void Line::InsertCityAfter(int index, City* city) {
    if (index < 0 || index >= static_cast<int>(points.size())) {
        DEBUG_ERROR("InsertCityAfter: Invalid handle index.");
        return;
    }

    // Create a new LinePoint for the city
    LinePoint newPoint;
    newPoint.isCity = true;
    newPoint.position = city->position;
    newPoint.city = city;

    // Insert the new point after the specified index
    points.insert(points.begin() + index + 1, newPoint);

    // Create and insert a new handle for this point
    handles.emplace(handles.begin() + index + 1, Handle(index + 1));

    // Update handle indices for subsequent handles
    for (size_t i = index + 2; i < handles.size(); ++i) {
        handles[i].index = static_cast<int>(i);
    }

    // Recompute Bezier segments to accommodate the new point
    UpdateBezierSegments();
}

void Line::AddNode(sf::Vector2f pos)
{
    LinePoint p;
    p.isCity = false;
    p.position = pos;
    p.city = nullptr;
    points.push_back(p);

    // Add a new handle for the node
    handles.emplace_back(handles.size());

    UpdateBezierSegments();
}

const std::vector<City*> Line::GetCities() const {
    std::vector<City*> cityList;
    for (const auto& point : points) {
        if (point.isCity && point.city != nullptr) {
            cityList.push_back(point.city);
        }
    }
    return cityList;
}

std::vector<sf::Vector2f> Line::GetPathPoints(int pointsPerSegment, std::vector<int>& cityIndices) const {
    std::vector<sf::Vector2f> path;
    cityIndices.clear();

    bool firstSegment = true;
    size_t pointIndex = 0; // Index for LinePoint

    for (const auto& segment : bezierSegments) {
        std::vector<sf::Vector2f> segmentPoints = ComputeCubicBezier(segment, pointsPerSegment);
        // Resample to equally spaced points
        std::vector<sf::Vector2f> resampledSegment = ResampleEquallySpaced(segmentPoints, pointsPerSegment);

        // Record the start of each segment as a city index
        if (!path.empty()) {
            resampledSegment.erase(resampledSegment.begin());
        }
        path.insert(path.end(), resampledSegment.begin(), resampledSegment.end());

        // Check if the end point of the segment is a city
        if (points.size() > pointIndex + 1) { // Ensure we don't go out of bounds
            if (points[pointIndex + 1].isCity) {
                cityIndices.push_back(path.size() - 1);
            }
        }

        if (firstSegment) {
            cityIndices.insert(cityIndices.begin(), 0);
            firstSegment = false;
        }

        pointIndex++;
    }
    return path;
}

sf::Vector2f Line::GetStartPosition() const {
    if (points.empty()) return sf::Vector2f(0.f, 0.f);
    return points.front().position;
}

sf::Vector2f Line::GetEndPosition() const {
    if (points.empty()) return sf::Vector2f(0.f, 0.f);
    return points.back().position;
}

sf::Vector2f Line::GetPointPosition(int index) const {
    if (index >= 0 && index < points.size()) {
        return points[index].position;
    }
    return sf::Vector2f(0.f, 0.f);
}

void Line::SelectHandle(int index) {
    DeselectHandles();
    for (auto& handle : handles) {
        if (handle.index == index) {
            handle.isSelected = true;
        }
    }
}

void Line::DeselectHandles() {
    for (auto& handle : handles) {
        handle.isSelected = false;
    }
}

int Line::GetSelectedHandleIndex() const {
    for (const auto& handle : handles) {
        if (handle.isSelected) {
            return handle.index;
        }
    }
    return -1; // No handle selected
}

std::vector<sf::Vector2f> Line::ComputeCubicBezier(const BezierSegment& segment, int numPoints) const {
    std::vector<sf::Vector2f> points;
    points.reserve(numPoints + 1);

    for (int i = 0; i <= numPoints; ++i) {
        float t = static_cast<float>(i) / numPoints;
        float u = 1.0f - t;

        // Cubic Bezier formula
        sf::Vector2f point = u * u * u * segment.start +
            3 * u * u * t * segment.startControl +
            3 * u * t * t * segment.endControl +
            t * t * t * segment.end;
        points.push_back(point);
    }

    return points;
}

std::vector<float> Line::CalculateCumulativeArcLength(const std::vector<sf::Vector2f>& points) const {
    std::vector<float> cumulativeLengths;
    cumulativeLengths.reserve(points.size());
    cumulativeLengths.push_back(0.0f);
    float totalLength = 0.0f;

    for (size_t i = 1; i < points.size(); ++i) {
        float segmentLength = std::sqrt(
            (points[i].x - points[i - 1].x) * (points[i].x - points[i - 1].x) +
            (points[i].y - points[i - 1].y) * (points[i].y - points[i - 1].y)
        );
        totalLength += segmentLength;
        cumulativeLengths.push_back(totalLength);
    }

    return cumulativeLengths;
}

std::vector<sf::Vector2f> Line::ResampleEquallySpaced(const std::vector<sf::Vector2f>& points, int numSamples) const {
    std::vector<sf::Vector2f> resampledPoints;
    std::vector<float> cumulativeLengths = CalculateCumulativeArcLength(points);
    float totalLength = cumulativeLengths.back();
    float interval = totalLength / (numSamples - 1);
    resampledPoints.reserve(numSamples);

    size_t currentSegment = 0;
    for (int i = 0; i < numSamples; ++i) {
        float targetLength = i * interval;
        while (currentSegment < cumulativeLengths.size() - 1 && cumulativeLengths[currentSegment + 1] < targetLength) {
            currentSegment++;
        }

        if (currentSegment >= cumulativeLengths.size() - 1) {
            resampledPoints.push_back(points.back());
            continue;
        }

        float segmentLength = cumulativeLengths[currentSegment + 1] - cumulativeLengths[currentSegment];
        float t = (targetLength - cumulativeLengths[currentSegment]) / segmentLength;
        sf::Vector2f newPoint = points[currentSegment] + t * (points[currentSegment + 1] - points[currentSegment]);
        resampledPoints.push_back(newPoint);
    }

    return resampledPoints;
}

void Line::UpdateBezierSegments() {
    bezierSegments.clear();
    size_t numPoints = points.size();
    if (numPoints < 2) return; // Need at least two cities to form a segment

    // Precompute directions for all cities
    std::vector<sf::Vector2f> directions(numPoints, sf::Vector2f(0.f, 0.f));
    for (size_t i = 0; i < numPoints; ++i) {
        if (i == 0) {
            // For the first city, direction is towards the next city
            directions[i] = points[i + 1].position - points[i].position;
        }
        else if (i == numPoints - 1) {
            // For the last city, direction is from the previous city
            directions[i] = points[i].position - points[i - 1].position;
        }
        else {
            // For middle cities, average the directions
            sf::Vector2f dirPrev = points[i].position - points[i - 1].position;
            sf::Vector2f dirNext = points[i + 1].position - points[i].position;
            directions[i] = Normalize(dirPrev) + Normalize(dirNext);
        }
        directions[i] = Normalize(directions[i]);
    }

    // Define an offset for control points
    float offset = 50.0f; // Adjust this value as needed for smoothness

    for (size_t i = 0; i < numPoints - 1; ++i) {
        BezierSegment segment;
        segment.start = points[i].position;
        segment.end = points[i + 1].position;

        // Set startControl based on the current city's direction
        segment.startControl = segment.start + directions[i] * offset;

        // Set endControl based on the next city's direction
        // If it's the last segment, use the current direction
        if (i + 1 < numPoints) {
            segment.endControl = segment.end - directions[i + 1] * offset;
        }
        else {
            // For the last segment, align the endControl with the current direction
            sf::Vector2f direction = Normalize(points[i + 1].position - points[i].position);
            segment.endControl = segment.end - direction * offset;
        }

        bezierSegments.push_back(segment);
    }
}

void Line::MoveHandle(int index, sf::Vector2f newPos)
{
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

    // Recompute the bezier segments to update the path
    UpdateBezierSegments();
}