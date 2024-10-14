// src/systems/ResourceManager.h
// Usage: auto& texture = ResourceManager<sf::Texture>::getInstance().load("player", "assets/images/player.png");

#pragma once
#include <map>
#include <string>
#include <memory>
#include <stdexcept>

template <typename Resource>
class ResourceManager {
public:
    static ResourceManager& getInstance() {
        static ResourceManager instance;
        return instance;
    }

    // Delete copy constructors
    ResourceManager(const ResourceManager&) = delete;
    void operator=(const ResourceManager&) = delete;

    Resource& load(const std::string& name, const std::string& filename);
    Resource& get(const std::string& name);

private:
    ResourceManager() = default;
    std::map<std::string, std::unique_ptr<Resource>> resources;
};
