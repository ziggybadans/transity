#include "Simulation.h"
#include "world/City.h"  // Assuming City type is defined here or elsewhere as needed
#include <iterator>

Simulation::Simulation(std::shared_ptr<Map> map)
    : m_map(std::move(map)), m_passengerSpawnTimer(0.0f) {}

void Simulation::Update(float scaledDt) {
    // Update all trains on the map.
    for (auto& train : m_map->GetTrains()) {
        train->Update(scaledDt);
    }

    // Update passengers on the map.
    m_map->GetCityManager().UpdatePassengers(scaledDt);

    // Handle periodic passenger spawning.
    m_passengerSpawnTimer += scaledDt;
    if (m_passengerSpawnTimer >= m_passengerSpawnInterval) {
        m_passengerSpawnTimer = 0.0f;

        auto& cities = m_map->GetCities();
        if (cities.size() >= 2) {
            auto it = cities.begin();
            City* origin = &(*it);
            std::advance(it, 1);
            City* destination = &(*it);

            // Spawn a passenger between two cities.
            m_map->GetCityManager().SpawnPassenger(origin, destination);
        }
    }
}
