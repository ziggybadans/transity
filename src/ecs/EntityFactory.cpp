#include "EntityFactory.h"
#include "Logger.h"
#include <SFML/Graphics/Color.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

EntityFactory::EntityFactory(entt::registry &registry) : _registry(registry) {
    LOG_INFO("EntityFactory", "EntityFactory created.");
    loadArchetypes("data/archetypes");
}

void EntityFactory::loadArchetypes(const std::string &directoryPath) {
    LOG_INFO("EntityFactory", "Loading archetypes from directory: %s", directoryPath.c_str());
    for (const auto &entry : std::filesystem::directory_iterator(directoryPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            std::ifstream file(entry.path());
            if (file.is_open()) {
                try {
                    nlohmann::json archetypeJson;
                    file >> archetypeJson;
                    if (archetypeJson.contains("id") && archetypeJson["id"].is_string()
                        && archetypeJson.contains("version")
                        && archetypeJson["version"].is_number()) {
                        const std::string id = archetypeJson["id"];
                        const int version = archetypeJson["version"];

                        // For now, we only support version 1.
                        // In a real application, you would add migration logic here.
                        if (version != 1) {
                            LOG_ERROR("EntityFactory", "Unsupported archetype version %d for '%s'",
                                      version, id.c_str());
                            continue;
                        }

                        _archetypes[id] = archetypeJson;
                        LOG_INFO("EntityFactory", "Loaded archetype: %s (Version: %d)", id.c_str(),
                                 version);
                    } else {
                        LOG_ERROR("EntityFactory",
                                  "Archetype file %s is missing 'id' or 'version' field, or they "
                                  "have an incorrect type.",
                                  entry.path().string().c_str());
                    }
                } catch (const nlohmann::json::exception &e) {
                    LOG_ERROR("EntityFactory", "Error parsing JSON from %s: %s",
                              entry.path().string().c_str(), e.what());
                }
                file.close();
            } else {
                LOG_ERROR("EntityFactory", "Could not open archetype file: %s",
                          entry.path().string().c_str());
            }
        }
    }
}

entt::entity EntityFactory::createEntity(const std::string &archetypeId,
                                         const sf::Vector2f &position, const std::string &name) {
    LOG_INFO("EntityFactory",
             "Request to create entity with archetype '%s' and name '%s' at (%.1f, %.1f).",
             archetypeId.c_str(), name.c_str(), position.x, position.y);

    auto it = _archetypes.find(archetypeId);
    if (it == _archetypes.end()) {
        LOG_ERROR("EntityFactory", "Archetype '%s' not found. Cannot create entity.",
                  archetypeId.c_str());
        return entt::null;
    }

    const nlohmann::json &archetypeJson = it->second;
    auto entity = _registry.create();
    _registry.emplace<PositionComponent>(entity, position);

    if (archetypeJson.contains("components") && archetypeJson["components"].is_object()) {
        const auto &components = archetypeJson["components"];
        for (auto &[componentName, componentData] : components.items()) {
            if (componentName == "renderable") {
                auto &renderable = _registry.emplace<RenderableComponent>(entity);
                if (componentData.contains("radius")) {
                    renderable.radius = {componentData["radius"].get<float>()};
                }
                if (componentData.contains("color") && componentData["color"].is_array()
                    && componentData["color"].size() == 4) {
                    renderable.color =
                        sf::Color(static_cast<unsigned char>(componentData["color"][0].get<int>()),
                                  static_cast<unsigned char>(componentData["color"][1].get<int>()),
                                  static_cast<unsigned char>(componentData["color"][2].get<int>()),
                                  static_cast<unsigned char>(componentData["color"][3].get<int>()));
                }
                if (componentData.contains("zOrder")) {
                    renderable.zOrder = {componentData["zOrder"].get<int>()};
                }
            } else if (componentName == "clickable") {
                auto &clickable = _registry.emplace<ClickableComponent>(entity);
                if (componentData.contains("boundingRadius")) {
                    clickable.boundingRadius = {componentData["boundingRadius"].get<float>()};
                }
            } else if (componentName == "city") {
                _registry.emplace<CityComponent>(entity);
            }
        }
    }

    LOG_DEBUG("EntityFactory", "Entity (ID: %u) created successfully using archetype '%s'.",
              static_cast<unsigned int>(entity), archetypeId.c_str());
    return entity;
}
entt::entity EntityFactory::createLine(const std::vector<entt::entity> &stops,
                                       const sf::Color &color) {
    LOG_INFO("EntityFactory", "Request to create line entity with %zu stops.", stops.size());
    if (stops.size() < 2) {
        LOG_ERROR("EntityFactory", "Cannot create line with less than 2 stops.");
        return entt::null;
    }

    auto entity = _registry.create();
    auto &lineComponent = _registry.emplace<LineComponent>(entity);
    lineComponent.stops = stops;
    lineComponent.color = color;

    LOG_DEBUG("EntityFactory", "Line entity (ID: %u) created successfully with %zu stops.",
              static_cast<unsigned int>(entity), stops.size());
    return entity;
}