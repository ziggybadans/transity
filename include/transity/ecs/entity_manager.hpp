#pragma once

#include <cstdint>
#include <queue>
#include <unordered_map>
#include <vector>

namespace transity::ecs {

using EntityID = std::uint32_t;
using EntityVersion = std::uint32_t;

/**
 * @brief Represents a unique entity identifier with versioning
 * 
 * The Entity struct combines an ID with a version number to ensure
 * that references to deleted entities can be detected.
 */
struct Entity {
    EntityID id;
    EntityVersion version;

    bool operator==(const Entity& other) const {
        return id == other.id && version == other.version;
    }
};

/**
 * @brief Manages entity lifecycle and validation in the ECS system
 * 
 * The EntityManager is responsible for:
 * - Creating and destroying entities
 * - Managing entity versioning
 * - Validating entity references
 * - Recycling entity IDs
 * - Providing iteration capabilities over active entities
 */
class EntityManager {
public:
    EntityManager();
    ~EntityManager() = default;

    // Prevent copying to ensure single source of truth
    EntityManager(const EntityManager&) = delete;
    EntityManager& operator=(const EntityManager&) = delete;

    /**
     * @brief Creates a new entity
     * @return Entity A new unique entity identifier
     */
    Entity createEntity();

    /**
     * @brief Destroys an entity and recycles its ID
     * @param entity The entity to destroy
     * @return bool True if the entity was valid and destroyed
     */
    bool destroyEntity(Entity entity);

    /**
     * @brief Checks if an entity is currently valid
     * @param entity The entity to validate
     * @return bool True if the entity exists and its version matches
     */
    bool isValid(Entity entity) const;

    /**
     * @brief Gets the current number of active entities
     * @return size_t The count of active entities
     */
    size_t getEntityCount() const;

    /**
     * @brief Gets a vector of all currently active entities
     * @return const std::vector<Entity>& Reference to the active entities vector
     */
    const std::vector<Entity>& getActiveEntities() const;

private:
    std::vector<Entity> activeEntities;              // Currently active entities
    std::queue<EntityID> recycledIds;               // IDs available for reuse
    std::unordered_map<EntityID, EntityVersion> versions;  // Current version for each ID
    EntityID nextEntityId;                          // Next ID to use if no recycled IDs
};

} // namespace transity::ecs 