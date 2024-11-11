#include "LineBuilder.h"
#include <iostream>

LineBuilder::LineBuilder()
    : currentLine(nullptr),
    isBuildingLine(false),
    isNextSegmentCurved(false),
    lineBeingExtended(nullptr),
    extendNodeIndex(-1)
{}

LineBuilder::~LineBuilder() {}

void LineBuilder::StartBuildingLine(Station* station) {
    currentLine = std::make_unique<Line>();
    currentLine->AddNode(station); // Add station node
    isBuildingLine = true;
    lineBeingExtended = nullptr;
    extendNodeIndex = -1;
}

void LineBuilder::AddNodeToCurrentLine(const sf::Vector2f& position) {
    if (isBuildingLine) {
        if (lineBeingExtended) {
            // Extending an existing line
            lineBeingExtended->AddNode(position);
        }
        else if (currentLine) {
            // Building a new line or branch
            currentLine->AddNode(position);
        }
    }
}

void LineBuilder::AddStationToCurrentLine(Station* station) {
    if (isBuildingLine) {
        if (lineBeingExtended) {
            // Extending an existing line with a station
            lineBeingExtended->AddNode(station);
        }
        else if (currentLine) {
            // Building a new line or branch with a station
            currentLine->AddNode(station);
        }
    }
}

void LineBuilder::FinishCurrentLine() {
    if (isBuildingLine) {
        if (lineBeingExtended && lineBeingExtended->IsActive()) {
            // Finished extending the existing line
            lineBeingExtended->SetActive(false);
            isBuildingLine = false;
            lineBeingExtended = nullptr;
            extendNodeIndex = -1;
        }
        else if (currentLine) {
            // Finished building a new line or branch
            currentLine->SetActive(false);
            isBuildingLine = false;
        }
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
    if (currentLine && isBuildingLine && !lineBeingExtended) {
        isBuildingLine = false;
        return std::move(currentLine);
    }
    // If extending, do not extract a new line
    return nullptr;
}

// Start building a branch line from an existing line node
void LineBuilder::StartBuildingBranch(Line* parentLine, const LineNode& startingNode) {
    currentLine = std::make_unique<Line>(parentLine);
    // Copy visual properties from parent line
    currentLine->SetColor(parentLine->GetColor());
    currentLine->SetThickness(parentLine->GetThickness());
    currentLine->SetSpeed(parentLine->GetSpeed());

    // Start the line from the given starting node
    if (startingNode.IsStation()) {
        currentLine->AddNode(startingNode.station);
    }
    else {
        currentLine->AddNode(startingNode.position);
    }

    isBuildingLine = true;
    lineBeingExtended = nullptr;
    extendNodeIndex = -1;
}

void LineBuilder::StartExtendingLine(Line* line, int nodeIndex) {
    if (!line) {
        std::cerr << "LineBuilder::StartExtendingLine called with null line." << std::endl;
        return;
    }
    if (nodeIndex != 0 && nodeIndex != static_cast<int>(line->GetNodes().size()) - 1) {
        std::cerr << "LineBuilder::StartExtendingLine called with invalid node index." << std::endl;
        return;
    }

    currentLine = nullptr; // We're extending an existing line, not creating a new one
    isBuildingLine = true;
    lineBeingExtended = line;
    extendNodeIndex = nodeIndex;
}

Line* LineBuilder::GetLineBeingExtended() const {
    return lineBeingExtended;
}

bool LineBuilder::IsExtendingLine() const {
    return lineBeingExtended != nullptr;
}

int LineBuilder::GetExtendNodeIndex() const {
    return extendNodeIndex;
}
