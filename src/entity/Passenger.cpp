#include "Passenger.h"
#include "../world/City.h"
#include "Train.h"
#include <algorithm>

Passenger::Passenger(City* origin, City* destination, const std::vector<City*>& route)
    : m_origin(origin), m_destination(destination), m_route(route), m_nextCityIndex(1),
    m_state(PassengerState::Waiting), m_currentTrain(nullptr)
{
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
