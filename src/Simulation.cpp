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
    m_map->UpdatePassengers(scaledDt);

    // Handle periodic passenger spawning.
    m_passengerSpawnTimer += scaledDt;
    if (m_passengerSpawnTimer >= m_passengerSpawnInterval) {
        m_passengerSpawnTimer = 0.0f;

        auto& cities = m_map->GetCities();
        if (cities.size() >= 2) {
            // Convert list to vector for easier random access
            std::vector<City*> cityPtrs;
            for (City& city : cities) {
                cityPtrs.push_back(&city);
            }

            // Set up random number generation
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(0, static_cast<int>(cityPtrs.size() - 1));

            // Select a random origin
            int originIndex = distrib(gen);
            City* origin = cityPtrs[originIndex];

            // Select a random destination ensuring it's not the same as origin
            City* destination = nullptr;
            do {
                int destIndex = distrib(gen);
                destination = cityPtrs[destIndex];
            } while (destination == origin);

            // Spawn a passenger between the randomly chosen cities.
            m_map->SpawnPassenger(origin, destination);
        }
    }
}
