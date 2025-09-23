#include "EntityFactory.h"
#include "Logger.h"
#include <SFML/Graphics/Color.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

EntityFactory::EntityFactory(entt::registry &registry) : _registry(registry) {
    LOG_DEBUG("EntityFactory", "EntityFactory created.");
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
                        LOG_DEBUG("EntityFactory", "Loaded archetype: %s (Version: %d)", id.c_str(),
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
    LOG_DEBUG("EntityFactory",
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

    if (_registry.all_of<PositionComponent>(entity)) {
        const auto &pos = _registry.get<PositionComponent>(entity);
        LOG_DEBUG("EntityFactory", "Entity %u has PositionComponent at (%.1f, %.1f)",
                  static_cast<unsigned int>(entity), pos.coordinates.x, pos.coordinates.y);
    }
    if (_registry.all_of<RenderableComponent>(entity)) {
        const auto &renderable = _registry.get<RenderableComponent>(entity);
        LOG_DEBUG("EntityFactory", "Entity %u has RenderableComponent with radius %.1f",
                  static_cast<unsigned int>(entity), renderable.radius.value);
    }

    if (!name.empty()) {
        _registry.emplace<NameComponent>(entity, name);
        LOG_DEBUG("EntityFactory", "Entity %u assigned name: %s", static_cast<unsigned int>(entity),
                  name.c_str());
    }

    return entity;
}
entt::entity EntityFactory::createLine(const std::vector<entt::entity> &stops,
                                       const sf::Color &color) {
    LOG_DEBUG("EntityFactory", "Request to create line entity with %zu stops.", stops.size());
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

    // createTrain(entity);

    return entity;
}

entt::entity EntityFactory::createTrain(entt::entity lineEntity) {
    if (!_registry.valid(lineEntity) || !_registry.all_of<LineComponent>(lineEntity)) {
        LOG_ERROR("EntityFactory", "Cannot create train for invalid line entity.");
        return entt::null;
    }

    const auto &line = _registry.get<LineComponent>(lineEntity);
    if (line.stops.empty()) {
        LOG_ERROR("EntityFactory", "Cannot create train for a line with no stops.");
        return entt::null;
    }

    const auto &firstStopPos = _registry.get<PositionComponent>(line.stops.front()).coordinates;

    // Use the archetype system to create the train
    std::string trainName = "Train " + std::to_string(entt::to_integral(lineEntity));
    auto trainEntity = createEntity("train", firstStopPos, trainName);
    if (trainEntity == entt::null) {
        LOG_ERROR("EntityFactory", "Failed to create train entity from archetype.");
        return entt::null;
    }

    _registry.emplace<TrainTag>(trainEntity);
    auto &movement = _registry.emplace<TrainMovementComponent>(trainEntity);
    movement.assignedLine = lineEntity;
    _registry.emplace<TrainPhysicsComponent>(trainEntity);
    _registry.emplace<TrainCapacityComponent>(trainEntity);

    LOG_DEBUG("EntityFactory", "Train entity (ID: %u) created for line (ID: %u).",
              static_cast<unsigned int>(trainEntity), static_cast<unsigned int>(lineEntity));

    return trainEntity;
}

entt::entity EntityFactory::createPassenger(entt::entity origin, entt::entity destination) {
    if (!_registry.valid(origin) || !_registry.valid(destination)) {
        LOG_ERROR("EntityFactory", "Cannot create passenger with invalid origin or destination.");
        return entt::null;
    }

    const auto &originPos = _registry.get<PositionComponent>(origin).coordinates;

    auto entity = _registry.create();
    _registry.emplace<PositionComponent>(entity, originPos);
    auto &passenger = _registry.emplace<PassengerComponent>(entity);
    passenger.originStation = origin;
    passenger.destinationStation = destination;
    passenger.currentContainer = origin;       // The passenger starts at the origin station
    _registry.emplace<PathComponent>(entity);  // Initially empty path

    // Add a RenderableComponent to make passengers visible
    auto &renderable = _registry.emplace<RenderableComponent>(entity);
    renderable.radius = {5.0f};  // Small circle for passengers
    renderable.color = sf::Color::Yellow;
    renderable.zOrder = {2};  // Render above lines but below stations/trains

    // Add a NameComponent
    std::string passengerName = "Passenger " + std::to_string(entt::to_integral(entity));
    _registry.emplace<NameComponent>(entity, passengerName);

    LOG_DEBUG("EntityFactory", "Passenger entity (ID: %u) created. Origin: %u, Destination: %u.",
              static_cast<unsigned int>(entity), static_cast<unsigned int>(origin),
              static_cast<unsigned int>(destination));

    return entity;
}