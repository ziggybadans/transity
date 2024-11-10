#pragma once

#include "../core/Line.h"
#include <memory>

class LineBuilder {
public:
    LineBuilder();
    ~LineBuilder();

    void StartBuildingLine(const sf::Vector2f& startPosition);
    void AddNodeToCurrentLine(const sf::Vector2f& position, bool isStation);
    void FinishCurrentLine();
    const Line* GetCurrentLine() const;
    bool IsBuildingLine() const;

    void SetNextSegmentCurved(bool curved);
    bool GetIsNextSegmentCurved() const;

    // Method to extract the current line and reset builder
    std::unique_ptr<Line> ExtractCurrentLine();

private:
    std::unique_ptr<Line> currentLine;
    bool isBuildingLine = false;
    bool isNextSegmentCurved = false;
};
