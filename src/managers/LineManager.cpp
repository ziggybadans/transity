#include "LineManager.h"
#include <cmath>

LineManager::LineManager() {}

LineManager::~LineManager() {}

void LineManager::AddLine(std::unique_ptr<Line> line) {
    lines.push_back(std::move(line));
}

const std::vector<std::unique_ptr<Line>>& LineManager::GetLines() const {
    return lines;
}

std::vector<std::unique_ptr<Line>>& LineManager::GetLines() {
    return lines;
}

Line* LineManager::GetLineAtPosition(const sf::Vector2f& position, float zoomLevel) {
    float tolerance = 5.0f * zoomLevel; // Adjust tolerance based on zoom level

    for (auto& linePtr : lines) {
        Line* line = linePtr.get();
        const auto& splinePoints = line->GetSplinePoints();
        for (size_t i = 0; i < splinePoints.size() - 1; ++i) {
            sf::Vector2f p1 = splinePoints[i];
            sf::Vector2f p2 = splinePoints[i + 1];

            // Calculate the closest point on the segment to the position
            sf::Vector2f delta = p2 - p1;
            float segmentLengthSquared = delta.x * delta.x + delta.y * delta.y;
            float t = ((position - p1).x * delta.x + (position - p1).y * delta.y) / segmentLengthSquared;
            t = std::max(0.0f, std::min(1.0f, t));
            sf::Vector2f projection = p1 + t * delta;

            float distanceSquared = (position - projection).x * (position - projection).x +
                (position - projection).y * (position - projection).y;

            if (distanceSquared <= tolerance * tolerance) {
                return line;
            }
        }
    }
    return nullptr;
}
