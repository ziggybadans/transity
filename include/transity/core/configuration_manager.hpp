#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>
#include "system.hpp"

namespace transity::core {

/**
 * @brief Configuration value type that can hold different types of settings
 */
using ConfigValue = std::variant<bool, int, double, std::string>;

/**
 * @brief Manages application configuration and user preferences
 * 
 * Handles loading, saving, and runtime modification of configuration settings.
 * Supports validation and migration of configuration data.
 */
class ConfigurationManager : public ISystem {
public:
    /**
     * @brief Get the singleton instance of the ConfigurationManager
     * @return Reference to the ConfigurationManager instance
     */
    static ConfigurationManager& getInstance();

    /**
     * @brief Initialize the configuration system
     * @param configPath Path to the configuration file
     * @return true if initialization successful, false otherwise
     */
    bool initialize(const std::filesystem::path& configPath = "config.json");

    /**
     * @brief Initialize the system (ISystem interface)
     * @return true if initialization successful, false otherwise
     */
    bool initialize() override { return initialize("config.json"); }

    /**
     * @brief Update the system (ISystem interface)
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime) override {}

    /**
     * @brief Clean up and save configuration on shutdown
     */
    void shutdown() override;

    /**
     * @brief Get the system name (ISystem interface)
     * @return The name of the system
     */
    std::string getName() const override { return "ConfigurationManager"; }

    /**
     * @brief Get a configuration value
     * @param key The configuration key
     * @param defaultValue Default value if key doesn't exist
     * @return The configuration value or default value
     */
    template<typename T>
    T getValue(const std::string& key, const T& defaultValue) const;

    /**
     * @brief Set a configuration value
     * @param key The configuration key
     * @param value The new value
     */
    template<typename T>
    void setValue(const std::string& key, const T& value);

    /**
     * @brief Save current configuration to file
     * @return true if save successful, false otherwise
     */
    bool saveConfiguration();

    /**
     * @brief Load configuration from file
     * @return true if load successful, false otherwise
     */
    bool loadConfiguration();

    /**
     * @brief Check if a configuration key exists
     * @param key The configuration key to check
     * @return true if key exists, false otherwise
     */
    bool hasKey(const std::string& key) const;

    /**
     * @brief Remove a configuration key
     * @param key The configuration key to remove
     */
    void removeKey(const std::string& key);

private:
    ConfigurationManager() = default;
    ~ConfigurationManager() = default;
    ConfigurationManager(const ConfigurationManager&) = delete;
    ConfigurationManager& operator=(const ConfigurationManager&) = delete;

    std::unordered_map<std::string, ConfigValue> configData;
    std::filesystem::path configFilePath;
    bool isDirty = false;

    bool validateConfiguration();
    void migrateConfiguration();
};

} // namespace transity::core 