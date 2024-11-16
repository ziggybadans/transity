#pragma once

#include <memory>
#include <mutex>
#include <condition_variable>
#include <string>
#include <unordered_map>
#include <SFML/Graphics.hpp>
#include "../world/WorldMap.h"
#include "../utility/ThreadPool.h"

class ResourceManager {
public:
    ResourceManager(std::unique_ptr<ThreadPool>& threadPool);
    ~ResourceManager() = default;

    bool LoadResources();
    std::shared_ptr<WorldMap> GetWorldMap() const { return m_worldMap; }

    template<typename T>
    std::shared_ptr<T> LoadResource(const std::string& key, const std::string& filePath);

private:
    template<typename T>
    void StoreResource(const std::string& key, std::shared_ptr<T> resource);

    std::unique_ptr<ThreadPool>& m_threadPool;
    std::shared_ptr<WorldMap> m_worldMap;
    std::mutex m_worldMapMutex;

    // Resource containers
    std::unordered_map<std::string, std::shared_ptr<sf::Texture>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<sf::Font>> m_fonts;
};

// Template implementation
template<typename T>
void ResourceManager::StoreResource(const std::string& key, std::shared_ptr<T> resource) {
    if constexpr (std::is_same_v<T, sf::Texture>) {
        m_textures[key] = resource;
    } else if constexpr (std::is_same_v<T, sf::Font>) {
        m_fonts[key] = resource;
    }
}

template<typename T>
std::shared_ptr<T> ResourceManager::LoadResource(const std::string& key, const std::string& filePath) {
    auto resource = std::make_shared<T>();
    if constexpr (std::is_same_v<T, sf::Texture>) {
        if (!resource->loadFromFile(filePath)) {
            std::cerr << "Failed to load texture: " << filePath << std::endl;
            return nullptr;
        }
    } else if constexpr (std::is_same_v<T, sf::Font>) {
        if (!resource->loadFromFile(filePath)) {
            std::cerr << "Failed to load font: " << filePath << std::endl;
            return nullptr;
        }
    }
    StoreResource(key, resource);
    return resource;
} 