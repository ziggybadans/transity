#include "LineManager.h"
#include "../../Debug.h"
#include "../City.h"
#include "../Line.h"
#include "../Map.h"
#include "../Segment.h"
#include "../Node.h"
#include <queue>

void LineManager::UseLineMode(const sf::Vector2f& pos) {
    DEBUG_DEBUG("Choosing to either create a new line or modify an existing line.");

    Line* selectedLine = m_map.GetSelectionManager().GetSelectedLine();

    if (selectedLine == nullptr) {
        // No line is currently selected; create a new line starting at 'pos'
        CreateLine(pos);
    }
    else {
        if (selectedLine->HasTrains()) {
            DEBUG_DEBUG("You need to remove any trains from the line before modifying it!");
            return;
        }

        // Retrieve the index of the selected handle (node)
        int selectedHandleIndex = selectedLine->GetSelectedHandleIndex();

        if (selectedHandleIndex != -1 &&
            selectedHandleIndex != 0 &&
            selectedHandleIndex != selectedLine->GetPointCount() - 1) {
            CreateBranch(selectedLine, selectedHandleIndex, pos);
            return;
        }

        // Attempt to find if a city was clicked
        City* clickedCity = m_map.FindCityAtPosition(pos);

        if (clickedCity) {
            if (selectedHandleIndex != -1) {
                // Handle is selected; determine where to add the new city relative to it
                if (selectedHandleIndex == 0) {
                    // Selected handle is the first node; add the city to the start
                    AddToLineStart(clickedCity->GetPosition());
                }
                else if (selectedHandleIndex == static_cast<int>(selectedLine->GetPoints().size() - 1)) {
                    // Selected handle is the last node; add the city to the end
                    AddToLineEnd(clickedCity->GetPosition());
                }
            }
            else {
                // No specific handle is selected; default action is to add to the end
                AddToLineEnd(clickedCity->GetPosition());
            }
        }
        else {
            Node* genericNode = m_map.FindGenericNodeAtPosition(pos);
            if (genericNode) {
                // Retrieve the currently selected line
                Line* line = m_map.GetSelectionManager().GetSelectedLine();
                if (!line) {
                    DEBUG_DEBUG("No line selected.");
                    return;
                }

                // Determine the starting point of the new segment: the last point on the line
                sf::Vector2f currentEnd = line->GetEndPosition();
                sf::Vector2f newNodePos = genericNode->GetPosition();

                // Check for parallel conflict with existing train lines
                if (m_map.WouldCauseParallelConflict(currentEnd, newNodePos)) {
                    DEBUG_DEBUG("Cannot add node. New segment would run parallel to an existing line with active trains.");
                    return;
                }

                // If no conflict, add the node to the line
                line->AddNode(genericNode);
                UpdateSharedSegments();
            }

            else {
                DEBUG_DEBUG("No valid city or generic node found at the clicked position.");
            }
        }
    }
}

void LineManager::CreateLine(const sf::Vector2f& pos) {
    DEBUG_DEBUG("Creating new line...");
    City* firstCity = nullptr;

    if (m_map.GetCities().empty()) {
        DEBUG_DEBUG("You need to create a city first!");
        return;
    }

    firstCity = m_map.FindCityAtPosition(pos);

    if (firstCity == nullptr) {
        DEBUG_DEBUG("You need to click on a city to create a line!");
        return;
    }

    static int lineSuffix = 1;
    std::string name = "Line" + std::to_string(lineSuffix++);

    m_lines.emplace_back(firstCity, name);
    Line* newLine = &m_lines.back();
    m_map.GetSelectionManager().SelectLine(newLine);

    UpdateSharedSegments();

    DEBUG_DEBUG("New line created originating from " + firstCity->GetName() + " with name " + name + ". Selected line has been updated for new line.");
}

void LineManager::AddToLineStart(const sf::Vector2f& pos) {
    DEBUG_DEBUG("Adding city to the start of line " + m_map.GetSelectionManager().GetSelectedLine()->GetName() + "...");

    City* newCity = m_map.FindCityAtPosition(pos);
    if (newCity == nullptr) {
        DEBUG_DEBUG("No valid city found at the clicked position to add to the line.");
        return;
    }

    // Check for parallel conflict before proceeding
    Line* selectedLine = m_map.GetSelectionManager().GetSelectedLine();
    if (!selectedLine) {
        DEBUG_DEBUG("No line selected.");
        return;
    }

    sf::Vector2f currentStart = selectedLine->GetStartPosition();
    if (m_map.WouldCauseParallelConflict(newCity->GetPosition(), currentStart)) {
        DEBUG_DEBUG("Cannot add city. New segment would run parallel to an existing line with active trains.");
        return;
    }

    // Check if the city is already part of the line
    auto cityList = m_map.GetSelectionManager().GetSelectedLine()->GetCities();
    if (std::find(cityList.begin(), cityList.end(), newCity) != cityList.end()) {
        DEBUG_DEBUG("The city is already part of the line.");
        return;
    }

    m_map.GetSelectionManager().GetSelectedLine()->AddCityToStart(newCity);
    UpdateSharedSegments();
    DEBUG_DEBUG("Added city with name " + newCity->GetName() + " to the start of line " + m_map.GetSelectionManager().GetSelectedLine()->GetName());

}

void LineManager::AddToLineEnd(const sf::Vector2f& pos) {
    DEBUG_DEBUG("Adding city to the end of line " + m_map.GetSelectionManager().GetSelectedLine()->GetName() + "...");

    City* newCity = m_map.FindCityAtPosition(pos);
    if (newCity == nullptr) {
        DEBUG_DEBUG("No valid city found at the clicked position to add to the line.");
        return;
    }

    // Check for parallel conflict before proceeding
    Line* selectedLine = m_map.GetSelectionManager().GetSelectedLine();
    if (!selectedLine) {
        DEBUG_DEBUG("No line selected.");
        return;
    }

    sf::Vector2f currentEnd = selectedLine->GetEndPosition();
    if (m_map.WouldCauseParallelConflict(currentEnd, newCity->GetPosition())) {
        DEBUG_DEBUG("Cannot add city. New segment would run parallel to an existing line with active trains.");
        return;
    }

    // Check if the city is already part of the line
    auto cityList = m_map.GetSelectionManager().GetSelectedLine()->GetCities();
    if (std::find(cityList.begin(), cityList.end(), newCity) != cityList.end()) {
        DEBUG_DEBUG("The city is already part of the line.");
        return;
    }

    m_map.GetSelectionManager().GetSelectedLine()->AddCityToEnd(newCity);
    UpdateSharedSegments();
    DEBUG_DEBUG("Added city with name " + newCity->GetName() + " to the end of line " + m_map.GetSelectionManager().GetSelectedLine()->GetName());

}

void LineManager::RemoveLine() {
    Line* selectedLine = m_map.GetSelectionManager().GetSelectedLine();
    if (selectedLine == nullptr) {
        DEBUG_DEBUG("No line selected. Cannot remove line.");
        return;
    }

    if (selectedLine->HasTrains()) {
        DEBUG_DEBUG("Cannot remove line. It has trains assigned.");
        return;
    }

    // Find the iterator to the selected line
    auto it = std::find_if(m_lines.begin(), m_lines.end(),
        [selectedLine](const Line& line) { return &line == selectedLine; });

    if (it != m_lines.end()) {
        DEBUG_DEBUG("Removing line: " + it->GetName());
        m_lines.erase(it);
        m_map.GetSelectionManager().DeselectAll();
        DEBUG_DEBUG("Line removed successfully.");
    }
    else {
        DEBUG_DEBUG("Selected line not found in the lines list.");
    }

}

void LineManager::MoveSelectedLineHandle(const sf::Vector2f& newPos) {
    Line* selectedLine = m_map.GetSelectionManager().GetSelectedLine();
    if (selectedLine == nullptr) {
        DEBUG_DEBUG("MoveSelectedLineHandle: No line selected.");
        return;
    }

    if (selectedLine->HasTrains()) {
        DEBUG_DEBUG("You need to remove any trains before you modify the line!");
        return;
    }

    int handleIndex = selectedLine->GetSelectedHandleIndex();
    if (handleIndex == -1) {
        DEBUG_DEBUG("MoveSelectedLineHandle: No handle is currently selected.");
        return;
    }

    // Move the handle
    selectedLine->MoveHandle(handleIndex, newPos);

}

void LineManager::UpdateSharedSegments() {
    // Temporary map to track segments and associated lines
    // Using sorted pair of positions as the key
    struct PositionPair {
        sf::Vector2f start;
        sf::Vector2f end;

        bool operator==(const PositionPair& other) const {
            return (ArePositionsEqual(start, other.start) && ArePositionsEqual(end, other.end)) ||
                (ArePositionsEqual(start, other.end) && ArePositionsEqual(end, other.start));
        }
    };

    struct PositionPairHash {
        std::size_t operator()(const PositionPair& pair) const {
            std::size_t h1 = std::hash<float>()(std::round(pair.start.x * 10.0f) / 10.0f);
            std::size_t h2 = std::hash<float>()(std::round(pair.start.y * 10.0f) / 10.0f);
            std::size_t h3 = std::hash<float>()(std::round(pair.end.x * 10.0f) / 10.0f);
            std::size_t h4 = std::hash<float>()(std::round(pair.end.y * 10.0f) / 10.0f);
            return h1 ^ (h2 << 1) ^ (h3 << 2) ^ (h4 << 3);
        }
    };

    std::unordered_map<PositionPair, std::vector<Line*>, PositionPairHash> segmentMap;

    // Iterate through all lines and their segments
    for (auto& line : m_lines) {
        const auto& pathPoints = line.GetPathPoints();
        for (size_t i = 0; i < pathPoints.size() - 1; ++i) {
            sf::Vector2f start = pathPoints[i];
            sf::Vector2f end = pathPoints[i + 1];

            // Create a sorted PositionPair to ensure consistency
            PositionPair pair;
            if (start.x < end.x || (start.x == end.x && start.y <= end.y)) {
                pair.start = start;
                pair.end = end;
            }
            else {
                pair.start = end;
                pair.end = start;
            }

            // Insert the line into the segment's line list
            segmentMap[pair].emplace_back(&line);
        }
    }

    // Clear existing shared segments
    sharedSegments.clear();

    // Populate sharedSegments with segments shared by multiple lines
    for (const auto& [pair, lines] : segmentMap) {
        if (lines.size() > 1) {
            // For each line that shares this segment, find the segment indices and create a Segment struct
            for (auto* line : lines) {
                const auto& pathPoints = line->GetPathPoints();
                for (size_t i = 0; i < pathPoints.size() - 1; ++i) {
                    if ((m_map.ArePositionsEqual(pathPoints[i], pair.start) && m_map.ArePositionsEqual(pathPoints[i + 1], pair.end)) ||
                        (m_map.ArePositionsEqual(pathPoints[i], pair.end) && m_map.ArePositionsEqual(pathPoints[i + 1], pair.start))) {
                        Segment seg(i, i + 1);
                        seg.overlappingLines = lines; // All lines sharing this segment
                        sharedSegments.emplace_back(seg);
                        break; // Move to next line
                    }
                }
            }
        }
    }

    // Now, for each line, calculate its offsets based on the shared segments
    for (auto& line : m_lines) {
        line.CalculateOffsets(sharedSegments);
    }

}

std::vector<Node*> LineManager::FindRouteBetweenNodes(Node* start, Node* end) {
    if (!start || !end) return {};

    // Build graph: node -> adjacent nodes
    std::unordered_map<Node*, std::vector<Node*>> graph;
    for (auto& line : m_lines) {
        const auto& points = line.GetPoints(); // Get all points (cities/nodes) on the line
        for (size_t i = 0; i + 1 < points.size(); ++i) {
            Node* A = points[i].node;
            Node* B = points[i + 1].node;
            graph[A].push_back(B);
            graph[B].push_back(A);
        }
    }

    // BFS setup
    std::queue<Node*> frontier;
    std::unordered_map<Node*, Node*> cameFrom;
    frontier.push(start);
    cameFrom[start] = nullptr;

    while (!frontier.empty()) {
        Node* current = frontier.front();
        frontier.pop();

        if (current == end) break;

        for (Node* next : graph[current]) {
            if (cameFrom.find(next) == cameFrom.end()) {
                frontier.push(next);
                cameFrom[next] = current;
            }
        }
    }

    // If end wasn't reached, return empty
    if (cameFrom.find(end) == cameFrom.end()) {
        return {};
    }

    // Reconstruct path from start to end
    std::vector<Node*> path;
    for (Node* at = end; at != nullptr; at = cameFrom[at]) {
        path.push_back(at);
    }
    std::reverse(path.begin(), path.end());
    return path;

}

void LineManager::CreateBranch(Line* parentLine, int branchHandleIndex, sf::Vector2f pos) {
    if (!parentLine) return;

    Node* branchStart = parentLine->GetNodeAt(branchHandleIndex);
    if (!branchStart) {
        DEBUG_DEBUG("CreateBranch: Invalid branch start node.");
        return;
    }

    static int lineSuffix = 1;
    std::string name = "Line" + std::to_string(lineSuffix++);

    // Create a new branch line starting from the branch point
    m_lines.emplace_back(branchStart, name);
    Line* newLine = &m_lines.back();
    m_map.GetSelectionManager().SelectLine(newLine);

    // Extend the new branch line based on the clicked position
    City* clickedCity = m_map.FindCityAtPosition(pos);
    if (clickedCity) {
        AddToLineEnd(clickedCity->GetPosition());
    }
    else {
        Node* genericNode = m_map.FindGenericNodeAtPosition(pos);
        if (genericNode) {
            Line* selectedLine = m_map.GetSelectionManager().GetSelectedLine();
            if (!selectedLine) {
                DEBUG_DEBUG("CreateBranch: No selected line after branch creation.");
                return;
            }
            sf::Vector2f currentEnd = selectedLine->GetEndPosition();
            sf::Vector2f newNodePos = genericNode->GetPosition();
            if (m_map.WouldCauseParallelConflict(currentEnd, newNodePos)) {
                DEBUG_DEBUG("Cannot add node to branch. New segment would run parallel to an existing line with active trains.");
                return;
            }
            selectedLine->AddNode(genericNode);
            UpdateSharedSegments();
        }
        else {
            DEBUG_DEBUG("No valid city or generic node found at the clicked position for branch extension.");
        }
    }

    UpdateSharedSegments();
}