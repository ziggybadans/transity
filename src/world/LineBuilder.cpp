#include "LineBuilder.h"

LineBuilder::LineBuilder() {}

LineBuilder::~LineBuilder() {}

void LineBuilder::StartBuildingLine(Station* station) {
    currentLine = std::make_unique<Line>();
    currentLine->AddNode(station); // Add station node
    isBuildingLine = true;
}

void LineBuilder::AddNodeToCurrentLine(const sf::Vector2f& position) {
    if (currentLine && isBuildingLine) {
        currentLine->AddNode(position);
    }
}

void LineBuilder::AddStationToCurrentLine(Station* station) {
    if (currentLine && isBuildingLine) {
        currentLine->AddNode(station);
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