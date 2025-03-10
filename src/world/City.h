#pragma once
#include "Node.h"
#include <vector>
#include <nlohmann/json.hpp>

class Passenger;  // Forward declaration

class City : public Node {
public:
    City(const std::string& cityName, const sf::Vector2f& cityPosition,
        unsigned int cityPopulation, float cityRadius = 10.0f)
        : Node(cityName, cityPosition, cityRadius), population(cityPopulation) {}

    nlohmann::json Serialize() const;
    void Deserialize(const nlohmann::json& j);

    unsigned int GetPopulation() const { return population; }

    bool operator==(const City& other) const {
        return GetName() == other.GetName() && GetPosition() == other.GetPosition();
    }

    void AddWaitingPassenger(Passenger* p) { 
        if (p && std::find(waitingPassengers.begin(), waitingPassengers.end(), p) == waitingPassengers.end()) {
            waitingPassengers.push_back(p);
        }
    }
    void RemoveWaitingPassenger(Passenger* p) {
        auto it = std::remove(waitingPassengers.begin(), waitingPassengers.end(), p);
        if (it != waitingPassengers.end()) {
            waitingPassengers.erase(it, waitingPassengers.end());
        }
    }
    const std::vector<Passenger*>& GetWaitingPassengers() const { return waitingPassengers; }

private:
    unsigned int population;
    std::vector<Passenger*> waitingPassengers;
};
