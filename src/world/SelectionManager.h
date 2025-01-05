// SelectionManager.h
#pragma once

#include "City.h"
#include "Line.h"
#include "../entity/Train.h"
#include <SFML/Graphics.hpp>

class SelectionManager {
public:
    SelectionManager()
        : selectedCity(nullptr), selectedLine(nullptr), selectedTrain(nullptr) {}

    // City Selection
    void SelectCity(City* city) {
        DeselectAll();
        selectedCity = city;
        if (selectedCity) {
            selectedCity->SetSelected(true);
        }
    }

    // Line Selection
    void SelectLine(Line* line) {
        DeselectAll();
        selectedLine = line;
        if (selectedLine) {
            selectedLine->SetSelected(true);
        }
    }

    // Train Selection
    void SelectTrain(Train* train) {
        DeselectAll();
        selectedTrain = train;
        if (selectedTrain) {
            selectedTrain->SetSelected(true);
        }
    }

    // Deselect All
    void DeselectAll() {
        if (selectedCity) {
            selectedCity->SetSelected(false);
        }
        if (selectedLine) {
            selectedLine->SetSelected(false);
        }
        if (selectedTrain) {
            selectedTrain->SetSelected(false);
        }
        selectedCity = nullptr;
        selectedLine = nullptr;
        selectedTrain = nullptr;
    }

    // Getters
    City* GetSelectedCity() const { return selectedCity; }
    Line* GetSelectedLine() const { return selectedLine; }
    Train* GetSelectedTrain() const { return selectedTrain; }

private:
    City* selectedCity;
    Line* selectedLine;
    Train* selectedTrain;
};
