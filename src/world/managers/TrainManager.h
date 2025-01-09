#pragma once
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>

class StateManager;
class City;
class Line;
class Map;
class Train;

class TrainManager {
public:
    TrainManager(Map& map, StateManager& stateMgr)
        : m_map(map), stateManager(stateMgr),
        startCityForTrain(nullptr), endCityForTrain(nullptr) {}

    void UseTrainPlaceMode(const sf::Vector2f& pos, bool left);
    void AddTrain();
    void RemoveTrain();
    std::vector<std::unique_ptr<Train>>& GetTrains() { return m_trains; }

    City* startCityForTrain;
    City* endCityForTrain;

private:
    std::vector<std::unique_ptr<Train>> m_trains;
    Map& m_map;
    StateManager& stateManager;
};
