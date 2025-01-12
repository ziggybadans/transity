#include "CityManager.h"
#include "../../Debug.h"
#include "../../entity/Passenger.h"
#include "../City.h"
#include "../Map.h"
#include <algorithm>
#include <cmath>

void CityManager::AddCity(const sf::Vector2f& pos) {
    /* Validation checks */
    // Check inside map bounds
    if (pos.x < 0 || pos.y < 0) { return; }
    if (pos.x >= static_cast<float>(m_map.GetSize()) * (Constants::TILE_SIZE * 0.98f) ||
        pos.y >= static_cast<float>(m_map.GetSize()) * (Constants::TILE_SIZE * 0.98f)) {
        return;
    }

    // Check outside minimum radius to another city
    for (const City& city : m_cities) {
        sf::Vector2f diff = city.GetPosition() - pos;
        float distanceSquared = diff.x * diff.x + diff.y * diff.y;
        if (distanceSquared <= static_cast<float>(m_minRadius * m_minRadius)) {
            return;
        }
    }

    /* Data generation */
    static int citySuffix = 1;
    std::string name = "City" + std::to_string(citySuffix++);
    unsigned int population = 1000;

    m_cities.emplace_back(name, pos, population); // Adds to the list of cities
}

void CityManager::RemoveCity(City* city) {
    City* selectedCity = m_map.GetSelectedCity();
    if (!selectedCity) {
        DEBUG_DEBUG("No city selected.");
        return;
    }

    // Check if the selected city is part of any line
    for (auto& line : m_map.GetLines()) {
        auto lineCities = line.GetCities();
        if (std::find(lineCities.begin(), lineCities.end(), selectedCity) != lineCities.end()) {
            DEBUG_DEBUG("City " + selectedCity->GetName() + " has lines running through it; cannot delete.");
            return;
        }
    }

    // Capture city name before deletion for logging purposes
    std::string cityName = selectedCity->GetName();

    // Clear selection before removing city to avoid dangling pointer issues
    m_map.DeselectAll();

    // Remove the city from the list
    m_cities.remove_if([selectedCity](const City& city) {
        return &city == selectedCity;
        });

    DEBUG_DEBUG("City " + cityName + " removed.");

}

void CityManager::MoveCity(const sf::Vector2f& newPos) {
    City* selectedCity = m_map.GetSelectedCity();
    if (!selectedCity) {
        DEBUG_DEBUG("No city selected.");
        return;
    }

    // Check if the city is part of any line
    for (auto& line : m_map.GetLines()) {
        auto lineCities = line.GetCities();
        if (std::find(lineCities.begin(), lineCities.end(), selectedCity) != lineCities.end()) {
            DEBUG_DEBUG("City " + selectedCity->GetName() + " has lines running through it; cannot move.");
            return;
        }
    }

    // Update the city's position
    selectedCity->SetPosition(newPos);
    DEBUG_DEBUG("City " + selectedCity->GetName() + " moved to new position.");

}

void CityManager::SpawnPassenger(City* origin, City* destination) {
    if (!origin || !destination || origin == destination) return;

    auto routeNodes = m_map.FindRouteBetweenNodes(origin, destination);
    if (routeNodes.empty()) return;

    std::vector<City*> routeCities;
    for (auto node : routeNodes) {
        // Filter only city nodes
        for (auto& city : m_cities) {
            if (&city == node) {
                routeCities.push_back(&city);
                break;
            }
        }
    }
    if (routeCities.size() < 2) return;

    // Create a passenger; the Passenger constructor handles adding to city's waiting list
    new Passenger(origin, destination, routeCities);

}

void CityManager::UpdatePassengers(float dt) {
    for (auto& train_ptr : m_map.GetTrains()) {
        Train* train = train_ptr.get();
        if (train->GetState() != "Waiting") continue;

        sf::Vector2f trainPos = train->GetPosition();
        City* currentCity = nullptr;
        for (auto& city : m_cities) {
            float dx = city.GetPosition().x - trainPos.x;
            float dy = city.GetPosition().y - trainPos.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist <= city.GetRadius() + 5.0f) {  // proximity threshold
                currentCity = &city;
                break;
            }
        }
        if (!currentCity) continue;

        std::vector<Passenger*> alightList;
        auto& passengersOnTrain = train->GetPassengers();
        for (auto p : passengersOnTrain) {
            if (p->GetDestination() == currentCity) {
                alightList.push_back(p);
                m_map.SetScore(m_map.GetScore() + 1);
            }
            else if (p->GetNextCity() == currentCity) {
                alightList.push_back(p);
            }
        }
        for (auto p : alightList) {
            train->RemovePassenger(p);
            if (p->GetState() == PassengerState::Arrived) {
                delete p;
            }
        }

        std::vector<Passenger*> waitingList = currentCity->GetWaitingPassengers();
        for (auto p : waitingList) {
            if (!train->HasCapacity()) break;
            if (p->GetState() == PassengerState::Waiting && p->GetCurrentCity() == currentCity) {
                bool goesToNext = false;
                const auto& stationPositions = train->GetStationPositions();
                float threshold = 5.0f;
                int indexCurrent = -1;
                int indexNext = -1;
                for (int i = 0; i < stationPositions.size(); ++i) {
                    float dcurr = std::hypot(stationPositions[i].x - currentCity->GetPosition().x,
                        stationPositions[i].y - currentCity->GetPosition().y);
                    float dnext = std::hypot(stationPositions[i].x - p->GetNextCity()->GetPosition().x,
                        stationPositions[i].y - p->GetNextCity()->GetPosition().y);
                    if (dcurr < threshold) indexCurrent = i;
                    if (dnext < threshold) indexNext = i;
                }
                if (indexCurrent != -1 && indexNext != -1 && indexNext > indexCurrent) {
                    goesToNext = true;
                }
                if (goesToNext) {
                    train->AddPassenger(p);
                }
            }
        }
    }

}
