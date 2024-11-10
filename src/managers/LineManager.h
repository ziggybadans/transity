#pragma once

#include "../core/Line.h"
#include <memory>
#include <vector>

class LineManager {
public:
    LineManager();
    ~LineManager();

    void AddLine(std::unique_ptr<Line> line);
    const std::vector<std::unique_ptr<Line>>& GetLines() const;
    std::vector<std::unique_ptr<Line>>& GetLines();

    Line* GetLineAtPosition(const sf::Vector2f& position, float zoomLevel);

private:
    // Containers to store lines
    std::vector<std::unique_ptr<Line>> lines;
};
