#pragma once

#include "../core/Line.h"
#include <memory>

class LineBuilder {
public:
    LineBuilder();
    ~LineBuilder();

    void StartBuildingLine(Station* station);
    void AddNodeToCurrentLine(const sf::Vector2f& position);
    void AddStationToCurrentLine(Station* station);
    void FinishCurrentLine();
    const Line* GetCurrentLine() const;
    bool IsBuildingLine() const;

    void SetNextSegmentCurved(bool curved);
    bool GetIsNextSegmentCurved() const;

    // Method to extract the current line and reset builder
    std::unique_ptr<Line> ExtractCurrentLine();

    // Method to start building a branch
    void StartBuildingBranch(Line* parentLine, const LineNode& startingNode);

private:
    std::unique_ptr<Line> currentLine;
    bool isBuildingLine = false;
    bool isNextSegmentCurved = false;
};
