#pragma once

#include "../City.h"
#include "../Line.h"
#include "../../entity/Train.h"
#include <SFML/Graphics.hpp>

class Map;

class SelectionManager {
public:
    SelectionManager(Map& map);

    void SelectObject(const sf::Vector2f& pos);
    bool SelectCity(sf::Vector2f pos);
    bool SelectLine(sf::Vector2f pos);
    bool SelectLineHandle(sf::Vector2f pos);
    bool SelectTrain(sf::Vector2f pos);

    void SelectCity(City* city);
    void SelectLine(Line* line);
    void SelectTrain(Train* train);
    void SelectNode(Node* node);

    // Deselect All
    void DeselectAll() {
        if (selectedCity) { selectedCity->SetSelected(false); }
        if (selectedLine) { selectedLine->SetSelected(false); }
        if (selectedTrain) { selectedTrain->SetSelected(false); }
        if (selectedNode) { selectedNode->SetSelected(false); }
        selectedCity = nullptr;
        selectedLine = nullptr;
        selectedTrain = nullptr;
        selectedNode = nullptr;
    }

    // Getters
    City* GetSelectedCity() const { return selectedCity; }
    Line* GetSelectedLine() const { return selectedLine; }
    Train* GetSelectedTrain() const { return selectedTrain; }
    Node* GetSelectedNode() const { return selectedNode; }

private:
    City* selectedCity;
    Line* selectedLine;
    Train* selectedTrain;
    Node* selectedNode;

    Map& m_map;
};
