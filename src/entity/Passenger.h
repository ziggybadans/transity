#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

class City;
class Train;

enum class PassengerState { Waiting, OnTrain, Arrived };

class Passenger {
public:
    Passenger(City* origin, City* destination, const std::vector<City*>& route);
    ~Passenger();

    nlohmann::json Serialize() const;
    void Deserialize(const nlohmann::json& j);

    std::string GetID() const { return m_id; }  // Getter for unique ID

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

    void ResolvePointers(const std::unordered_map<std::string, City*>& cityLookup);

private:
    static size_t s_nextId;  // Static counter for generating unique IDs
    std::string m_id;        // Unique identifier for this passenger

    City* m_origin;
    City* m_destination;
    City* m_currentCity;
    Train* m_currentTrain;
    PassengerState m_state;
    std::vector<City*> m_route;
    size_t m_nextCityIndex;

    std::string m_originName;
    std::string m_destinationName;
    std::string m_currentCityName;
    std::vector<std::string> m_routeNames;
};
