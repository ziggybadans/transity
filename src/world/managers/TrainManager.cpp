#include "TrainManager.h"
#include "../../Debug.h"
#include "../Map.h"
#include "../City.h"
#include "../Line.h"
#include "../../entity/Train.h"

void TrainManager::UseTrainPlaceMode(const sf::Vector2f& pos, bool left) {
    City* clickedCity = m_map.FindCityAtPosition(pos);
    if (clickedCity) {
        if (left) {
            startCityForTrain = clickedCity;
        }
        else {
            endCityForTrain = clickedCity;
        }
    }

    if (startCityForTrain != nullptr && endCityForTrain != nullptr) {
        stateManager.SetState("TrainPlaceVerified", true);
    }
}

void TrainManager::AddTrain() {
    // Removed check for a selected line since multi-line functionality no longer requires it.
    if (startCityForTrain == nullptr || endCityForTrain == nullptr) {
        DEBUG_ERROR("AddTrain: Start or end city not selected.");
        return;
    }

    std::vector<Node*> routeNodes = m_map.GetLineManager().FindRouteBetweenNodes(startCityForTrain, endCityForTrain);
    if (routeNodes.empty()) {
        DEBUG_ERROR("AddTrain: No route found between selected cities.");
        return;
    }

    std::vector<sf::Vector2f> fullPathPoints;
    Line* firstLine = nullptr;
    std::vector<sf::Vector2f> allStations;
    allStations.reserve(64);

    // Compute the multi-line path and station list
    for (size_t i = 0; i + 1 < routeNodes.size(); ++i) {
        Node* nodeA = routeNodes[i];
        Node* nodeB = routeNodes[i + 1];
        Line* connectingLine = nullptr;

        for (auto& line : m_map.GetLines()) {
            const auto& points = line.GetPoints();
            for (size_t j = 0; j + 1 < points.size(); ++j) {
                if ((points[j].node == nodeA && points[j + 1].node == nodeB) ||
                    (points[j].node == nodeB && points[j + 1].node == nodeA)) {
                    connectingLine = &line;
                    break;
                }
            }
            if (connectingLine) break;
        }

        if (!connectingLine) {
            DEBUG_ERROR("AddTrain: No connecting line found between nodes.");
            return;
        }

        const auto& linePoints = connectingLine->GetPathPoints();
        const auto& cityIndices = connectingLine->GetCityIndices();
        int idxA = -1, idxB = -1;
        for (size_t j = 0; j < linePoints.size(); ++j) {
            if (m_map.ArePositionsEqual(linePoints[j], nodeA->GetPosition())) idxA = (int)j;
            if (m_map.ArePositionsEqual(linePoints[j], nodeB->GetPosition())) idxB = (int)j;
        }
        if (idxA == -1 || idxB == -1) {
            DEBUG_ERROR("AddTrain: Could not locate segment in connecting line.");
            return;
        }

        int startIdx = std::min(idxA, idxB);
        int endIdx = std::max(idxA, idxB);
        for (int k = startIdx; k <= endIdx; ++k) {
            fullPathPoints.push_back(linePoints[k]);
        }

        for (int cIdx : cityIndices) {
            sf::Vector2f cPos = connectingLine->GetPointPosition(cIdx);
            bool alreadyAdded = false;
            for (auto& st : allStations) {
                float dist = std::hypot(st.x - cPos.x, st.y - cPos.y);
                if (dist < 0.1f) {
                    alreadyAdded = true;
                    break;
                }
            }
            if (!alreadyAdded) {
                allStations.push_back(cPos);
            }
        }

        if (i == 0) {
            firstLine = connectingLine;
        }
    }

    if (fullPathPoints.empty()) {
        DEBUG_ERROR("AddTrain: Constructed path is empty.");
        return;
    }

    // Remove consecutive duplicate points from fullPathPoints
    std::vector<sf::Vector2f> filteredPathPoints;
    if (!fullPathPoints.empty()) {
        filteredPathPoints.push_back(fullPathPoints.front());
        for (size_t i = 1; i < fullPathPoints.size(); ++i) {
            if (!m_map.ArePositionsEqual(fullPathPoints[i], fullPathPoints[i - 1])) {
                filteredPathPoints.push_back(fullPathPoints[i]);
            }
        }
    }
    fullPathPoints = filteredPathPoints;

    std::string trainID = "Train" + std::to_string(m_trains.size() + 1);
    std::unique_ptr<Train> newTrain = std::make_unique<Train>(
        firstLine,
        trainID,
        fullPathPoints,
        allStations
    );

    if (firstLine) {
        firstLine->AddTrain(newTrain.get());
    }
    m_trains.push_back(std::move(newTrain));

    startCityForTrain = nullptr;
    endCityForTrain = nullptr;

    DEBUG_DEBUG("Added " + trainID + " with multi-line route. Station list size: " + std::to_string(allStations.size()));

}

void TrainManager::RemoveTrain() {
    Train* selectedTrain = m_map.GetSelectionManager().GetSelectedTrain();
    if (!selectedTrain) { return; }

    // Remove train from the line
    Line* route = selectedTrain->GetRoute();
    if (route) {
        route->RemoveTrain(selectedTrain);
    }

    // Erase from the vector
    m_trains.erase(
        std::remove_if(m_trains.begin(), m_trains.end(),
            [selectedTrain](const std::unique_ptr<Train>& tptr) {
                return tptr.get() == selectedTrain;
            }
        ),
        m_trains.end()
    );

    // Deselect train
    m_map.GetSelectionManager().DeselectAll();
}
