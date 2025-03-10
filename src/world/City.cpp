#include "City.h"
#include "../entity/Passenger.h"

nlohmann::json City::Serialize() const {
    nlohmann::json j = Node::Serialize();  // Serialize base Node data
    j["population"] = population;
    // Serialize waiting passengers by their identifiers (if applicable)
    j["waitingPassengers"] = nlohmann::json::array();
    for (auto* p : waitingPassengers) {
        // Assuming Passenger has a method to get a unique ID
        j["waitingPassengers"].push_back(p->GetID());
    }
    return j;
}

void City::Deserialize(const nlohmann::json& j) {
    Node::Deserialize(j);
    population = j["population"].get<unsigned int>();
    // Waiting passengers will be resolved after all objects are loaded
    // For now, you may leave waitingPassengers empty or store temporary IDs for later resolution
}