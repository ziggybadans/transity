#pragma once

#include <entt/entt.hpp>

namespace transity::ecs {

class ECSCore {
public:
    void initialize();
    void shutdown();
private:
    entt::registry _registry;
};

}