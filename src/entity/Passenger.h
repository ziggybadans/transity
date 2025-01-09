#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class City;
class Train;

enum class PassengerState { Waiting, OnTrain, Arrived };

class Passenger {
public:
    Passenger(City* origin, City* destination, const std::vector<City*>& route);
    ~Passenger();

    City* GetCurrentCity() const;
    City* GetOrigin() const { return m_origin; }
    City* GetDestination() const;
    PassengerState GetState() const;
    const std::vector<City*>& GetRoute() const;
    size_t GetNextCityIndex() const;
    City* GetNextCity() const;

    void BoardTrain(Train* train);
    void AlightAtCity(City* city);
    void Arrive();

private:
    City* m_origin;
    City* m_destination;
    City* m_currentCity;
    Train* m_currentTrain;
    PassengerState m_state;
    std::vector<City*> m_route;
    size_t m_nextCityIndex;
};
