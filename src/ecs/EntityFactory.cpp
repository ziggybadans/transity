#include "EntityFactory.h"
#include "Constants.h"
#include "Logger.h"
#include <SFML/Graphics/Color.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

EntityFactory::EntityFactory(entt::registry &registry, const std::string &archetypesPath)
    : _registry(registry) {
    LOG_DEBUG("EntityFactory", "EntityFactory created.");
    registerComponentFactories();
    loadArchetypes(archetypesPath);
}

void EntityFactory::registerComponentFactories() {
    _componentFactories["renderable"] = [this](entt::entity entity, const nlohmann::json &data) {
        auto &renderable = _registry.emplace<RenderableComponent>(entity);
        if (data.contains("radius")) renderable.radius = {data["radius"].get<float>()};
        if (data.contains("color") && data["color"].is_array() && data["color"].size() == 4) {
            renderable.color = sf::Color(data["color"][0].get<int>(), data["color"][1].get<int>(),
                                         data["color"][2].get<int>(), data["color"][3].get<int>());
        }
        if (data.contains("zOrder")) renderable.zOrder = {data["zOrder"].get<int>()};
    };
    _componentFactories["clickable"] = [this](entt::entity entity, const nlohmann::json &data) {
        auto &clickable = _registry.emplace<ClickableComponent>(entity);
        if (data.contains("boundingRadius"))
            clickable.boundingRadius = {data["boundingRadius"].get<float>()};
    };
    _componentFactories["city"] = [this](entt::entity entity, const nlohmann::json &data) {
        _registry.emplace<CityComponent>(entity);
    };
    _componentFactories["train"] = [this](entt::entity entity, const nlohmann::json &data) {
        _registry.emplace<TrainTag>(entity);
    };
    _componentFactories["trainMovement"] = [this](entt::entity entity, const nlohmann::json &data) {
        _registry.emplace<TrainMovementComponent>(entity);
    };
    _componentFactories["trainPhysics"] = [this](entt::entity entity, const nlohmann::json &data) {
        _registry.emplace<TrainPhysicsComponent>(entity);
    };
    _componentFactories["trainCapacity"] = [this](entt::entity entity, const nlohmann::json &data) {
        auto &capacity = _registry.emplace<TrainCapacityComponent>(entity);
        if (data.contains("capacity")) capacity.capacity = data["capacity"].get<int>();
    };
    _componentFactories["passenger"] = [this](entt::entity entity, const nlohmann::json &data) {
        _registry.emplace<PassengerComponent>(entity);
    };
    _componentFactories["path"] = [this](entt::entity entity, const nlohmann::json &data) {
        _registry.emplace<PathComponent>(entity);
    };
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

                        if (version != Constants::SUPPORTED_ARCHETYPE_VERSION) {
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
            auto factoryIt = _componentFactories.find(componentName);
            if (factoryIt != _componentFactories.end()) {
                factoryIt->second(entity, componentData);
            } else {
                LOG_WARN("EntityFactory", "No factory found for component '%s' in archetype '%s'.",
                         componentName.c_str(), archetypeId.c_str());
            }
        }
    }

    LOG_DEBUG("EntityFactory", "Entity (ID: %u) created successfully using archetype '%s'.",
              static_cast<unsigned int>(entity), archetypeId.c_str());

    if (!name.empty()) {
        _registry.emplace<NameComponent>(entity, name);
        LOG_TRACE("EntityFactory", "Entity %u assigned name: %s", static_cast<unsigned int>(entity),
                  name.c_str());
    }

    return entity;
}

entt::entity EntityFactory::createEntity(const std::string &archetypeId,
                                         const sf::Vector2f &position, CityType cityType,
                                         const std::string &name) {
    auto entity = createEntity(archetypeId, position, name);
    if (entity != entt::null) {
        if (_registry.all_of<CityComponent>(entity)) {
            auto &cityComponent = _registry.get<CityComponent>(entity);
            cityComponent.type = cityType;
        } else {
            _registry.emplace<CityComponent>(entity, cityType);
        }
    }
    return entity;
}

entt::entity EntityFactory::createLine(const std::vector<LinePoint> &points,
                                       const sf::Color &color) {
    LOG_DEBUG("EntityFactory", "Request to create line entity with %zu points.", points.size());
    if (points.size() < 2) {
        LOG_ERROR("EntityFactory", "Cannot create line with less than 2 points.");
        return entt::null;
    }

    auto entity = _registry.create();
    auto &lineComponent = _registry.emplace<LineComponent>(entity);
    lineComponent.points = points;
    lineComponent.color = color;

    LOG_DEBUG("EntityFactory", "Line entity (ID: %u) created successfully with %zu points.",
              static_cast<unsigned int>(entity), points.size());

    return entity;
}

entt::entity EntityFactory::createTrain(entt::entity lineEntity) {
    if (!_registry.valid(lineEntity) || !_registry.all_of<LineComponent>(lineEntity)) {
        LOG_ERROR("EntityFactory", "Cannot create train for invalid line entity.");
        return entt::null;
    }

    const auto &line = _registry.get<LineComponent>(lineEntity);
    if (line.points.empty()) {
        LOG_ERROR("EntityFactory", "Cannot create train for a line with no stops.");
        return entt::null;
    }

    const auto &firstStopPos = line.points.front().position;

    std::string trainName = "Train " + std::to_string(entt::to_integral(lineEntity));
    auto trainEntity = createEntity("train", firstStopPos, trainName);
    if (trainEntity == entt::null) {
        LOG_ERROR("EntityFactory", "Failed to create train entity from archetype.");
        return entt::null;
    }

    auto &movement = _registry.get<TrainMovementComponent>(trainEntity);
    movement.assignedLine = lineEntity;

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

    std::string passengerName = "Passenger " + std::to_string(entt::to_integral(origin)) + "->"
                                + std::to_string(entt::to_integral(destination));
    auto entity = createEntity("passenger", originPos, passengerName);
    if (entity == entt::null) {
        LOG_ERROR("EntityFactory", "Failed to create passenger entity from archetype.");
        return entt::null;
    }

    auto &passenger = _registry.get<PassengerComponent>(entity);
    passenger.originStation = origin;
    passenger.destinationStation = destination;
    passenger.currentContainer = origin;

    LOG_DEBUG("EntityFactory", "Passenger entity (ID: %u) created. Origin: %u, Destination: %u.",
              static_cast<unsigned int>(entity), static_cast<unsigned int>(origin),
              static_cast<unsigned int>(destination));

    return entity;
}