#include "ecs/ECSCore.hpp"
#include "logging/LoggingSystem.hpp"

namespace transity::ecs {

void ECSCore::initialize() {
    LOG_INFO("ECS", "ECS core initialized");
}

void ECSCore::shutdown() {
}

}