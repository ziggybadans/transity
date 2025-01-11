#include "Passenger.h"
#include "../world/City.h"
#include "Train.h"
#include <algorithm>

size_t Passenger::s_nextId = 0;

Passenger::Passenger(City* origin, City* destination, const std::vector<City*>& route)
    : m_origin(origin), m_destination(destination), m_route(route), m_nextCityIndex(1),
    m_state(PassengerState::Waiting), m_currentTrain(nullptr)
{
    // Assign a unique ID using the static counter
    m_id = "passenger_" + std::to_string(s_nextId++);

    m_currentCity = origin;
    if (m_origin) m_origin->AddWaitingPassenger(this);
}

Passenger::~Passenger() {
    if (m_state == PassengerState::Waiting && m_currentCity) {
        m_currentCity->RemoveWaitingPassenger(this);
    }
}

City* Passenger::GetCurrentCity() const { return m_currentCity; }
City* Passenger::GetDestination() const { return m_destination; }
PassengerState Passenger::GetState() const { return m_state; }
const std::vector<City*>& Passenger::GetRoute() const { return m_route; }
size_t Passenger::GetNextCityIndex() const { return m_nextCityIndex; }
City* Passenger::GetNextCity() const {
    if (m_nextCityIndex < m_route.size()) return m_route[m_nextCityIndex];
    return nullptr;
}

void Passenger::BoardTrain(Train* train) {
    if (m_state != PassengerState::Waiting) return;
    m_currentTrain = train;
    m_state = PassengerState::OnTrain;
    if (m_currentCity) {
        m_currentCity->RemoveWaitingPassenger(this);
        m_currentCity = nullptr;
    }
}

void Passenger::AlightAtCity(City* city) {
    if (m_state != PassengerState::OnTrain) return;
    m_currentTrain = nullptr;
    m_currentCity = city;
    m_state = PassengerState::Waiting;
    if (city) city->AddWaitingPassenger(this);

    // Advance to next leg of journey if possible
    if (m_nextCityIndex + 1 < m_route.size())
        ++m_nextCityIndex;
}

void Passenger::Arrive() {
    m_state = PassengerState::Arrived;
    if (m_currentTrain) {
        m_currentTrain = nullptr;
    }
    if (m_currentCity) {
        m_currentCity->RemoveWaitingPassenger(this);
        m_currentCity = nullptr;
    }
}

nlohmann::json Passenger::Serialize() const {
    nlohmann::json j;
    j["id"] = m_id;  // Include unique ID in serialization
    j["origin"] = m_origin ? m_origin->GetName() : "";
    j["destination"] = m_destination ? m_destination->GetName() : "";
    j["currentCity"] = m_currentCity ? m_currentCity->GetName() : "";
    j["state"] = static_cast<int>(m_state);
    j["route"] = nlohmann::json::array();
    for (auto* city : m_route) {
        j["route"].push_back(city->GetName());
    }
    j["nextCityIndex"] = m_nextCityIndex;
    return j;
}

void Passenger::Deserialize(const nlohmann::json& j) {
    m_id = j["id"].get<std::string>();
    m_originName = j["origin"].get<std::string>();
    m_destinationName = j["destination"].get<std::string>();
    m_currentCityName = j["currentCity"].get<std::string>();
    m_state = static_cast<PassengerState>(j["state"].get<int>());
    m_routeNames.clear();
    for (auto& cityName : j["route"]) {
        m_routeNames.push_back(cityName.get<std::string>());
    }
    m_nextCityIndex = j["nextCityIndex"].get<size_t>();
}

void Passenger::ResolvePointers(const std::unordered_map<std::string, City*>& cityLookup) {
    if (cityLookup.find(m_originName) != cityLookup.end()) {
        m_origin = cityLookup.at(m_originName);
    }
    if (cityLookup.find(m_destinationName) != cityLookup.end()) {
        m_destination = cityLookup.at(m_destinationName);
    }
    if (!m_currentCityName.empty() && cityLookup.find(m_currentCityName) != cityLookup.end()) {
        m_currentCity = cityLookup.at(m_currentCityName);
    }
    m_route.clear();
    for (auto& name : m_routeNames) {
        if (cityLookup.find(name) != cityLookup.end()) {
            m_route.push_back(cityLookup.at(name));
        }
    }
    // Clear temporary name storage if no longer needed
    m_originName.clear();
    m_destinationName.clear();
    m_currentCityName.clear();
    m_routeNames.clear();
}