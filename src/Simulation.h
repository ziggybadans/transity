#pragma once

#include <memory>
#include <random>
#include "world/Map.h"

class Simulation {
public:
    explicit Simulation(std::shared_ptr<Map> map);
    void Update(float scaledDt);

private:
    std::shared_ptr<Map> m_map;
    float m_passengerSpawnTimer;
    const float m_passengerSpawnInterval = 10.0f; // spawn a new passenger every 10 seconds
};
