#include "LineBuilder.h"

LineBuilder::LineBuilder() {}

LineBuilder::~LineBuilder() {}

void LineBuilder::StartBuildingLine(const sf::Vector2f& startPosition) {
    currentLine = std::make_unique<Line>();
    currentLine->AddNode(startPosition, true); // Pass 'true' to indicate it's a station
    isBuildingLine = true;
}

void LineBuilder::AddNodeToCurrentLine(const sf::Vector2f& position, bool isStation) {
    if (currentLine && isBuildingLine) {
        currentLine->AddNode(position, isStation);
    }
}

void LineBuilder::FinishCurrentLine() {
    if (currentLine) {
        currentLine->SetActive(false);
        isBuildingLine = false;
    }
}

const Line* LineBuilder::GetCurrentLine() const {
    return isBuildingLine ? currentLine.get() : nullptr;
}

bool LineBuilder::IsBuildingLine() const {
    return isBuildingLine;
}

void LineBuilder::SetNextSegmentCurved(bool curved) {
    isNextSegmentCurved = curved;
}

bool LineBuilder::GetIsNextSegmentCurved() const {
    return isNextSegmentCurved;
}

std::unique_ptr<Line> LineBuilder::ExtractCurrentLine() {
    if (currentLine && isBuildingLine) {
        isBuildingLine = false;
        return std::move(currentLine);
    }
    return nullptr;
}
