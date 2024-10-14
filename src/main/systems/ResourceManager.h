// src/systems/ResourceManager.h
// Usage: auto& texture = ResourceManager<sf::Texture>::getInstance().load("player", "assets/images/player.png");

#pragma once
#include <map>       // Include for storing resources with string keys
#include <string>    // Include for handling resource names and file paths
#include <memory>    // Include for managing resources with smart pointers
#include <stdexcept> // Include for throwing exceptions if a resource is not found

// Template class for managing resources such as textures, fonts, or sounds
template <typename Resource>
class ResourceManager {
public:
    // Loads a resource from a file and stores it with a given name
    Resource& load(const std::string& name, const std::string& filename) {
        auto it = resources.find(name);
        if (it != resources.end()) {
            return *(it->second); // Return existing resource if it is already loaded
        }

        auto resource = std::make_unique<Resource>();
        if (!resource->loadFromFile(filename)) { // Load resource from file
            throw std::runtime_error("ResourceManager: Failed to load " + filename);
        }

        resources[name] = std::move(resource); // Store the loaded resource in the map
        return *resources[name];
    }

    // Retrieves a resource by name
    Resource& get(const std::string& name) {
        auto it = resources.find(name);
        if (it == resources.end()) {
            throw std::runtime_error("ResourceManager: Resource not found: " + name);
        }
        return *(it->second); // Return the requested resource
    }

    // Singleton access if using Singleton pattern
    static ResourceManager& getInstance() {
        static ResourceManager instance;
        return instance;
    }

    // Delete copy constructors to prevent copying the singleton
    ResourceManager(const ResourceManager&) = delete;
    void operator=(const ResourceManager&) = delete;

private:
    ResourceManager() = default; // Private constructor for Singleton pattern
    std::map<std::string, std::unique_ptr<Resource>> resources; // Map to store resources with unique pointers
};

// Summary:
// The ResourceManager class is a template-based singleton that helps manage game resources like textures, sounds, or fonts.
// It provides methods to load resources from files and retrieve them by name, ensuring each resource is loaded only once.
// The Singleton pattern ensures there is a single instance of ResourceManager, making resource access consistent and efficient.