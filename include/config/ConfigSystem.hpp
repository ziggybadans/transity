#pragma once

#include <string>
#include <map>
#include <any>
#include <mutex>

#include <toml++/toml.hpp>
#include "logging/LoggingSystem.hpp"

namespace transity::config {

class ConfigSystem {
public:
    ConfigSystem();
    ~ConfigSystem();

    void initialize(const std::string& primaryConfigFilepath = "config.toml",
                    const std::string& userConfigFilepath = "");

    inline std::string getString(const std::string& key, const std::string& defaultValue = "") const {
        return getValue<std::string>(key, defaultValue);
    }

    inline int getInt(const std::string& key, int defaultValue = 0) const {
        return getValue<int>(key, defaultValue);
    }

    inline bool getBool(const std::string& key, bool defaultValue = false) const {
        return getValue<bool>(key, defaultValue);
    }

    inline double getDouble(const std::string& key, double defaultValue = 0.0) const {
        return getValue<double>(key, defaultValue);
    }

    inline float getFloat(const std::string& key, float defaultValue = 0.0f) const {
        return static_cast<float>(getValue<double>(key, static_cast<double>(defaultValue)));
    }

    inline std::vector<std::string> splitPath(const std::string& s, char delimiter) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(s);
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    template <typename T>
    T getValue(const std::string& key, T defaultValue) const {
        std::lock_guard<std::mutex> lock(configMutex_);
        std::optional<T> result;

        result = tryGetValueFromSource<T>(runtimeOverrides, key);
        if (result) return *result;

        result = tryGetValueFromSource<T>(userConfigTable, key);
        if (result) return *result;

        result = tryGetValueFromSource<T>(primaryConfigTable, key);
        if (result) return *result;

        result = tryGetValueFromSource<T>(defaultConfigValues, key);
        if (result) return *result;

        LOG_WARN(("Config key '" + key + "' not found or type mismatch in any config source. Returning default.").c_str(), "Config");
        return defaultValue;
    }

    template <typename T>
    void setValue(const std::string& key, T value) {
        std::lock_guard<std::mutex> lock(configMutex_);
        std::vector<std::string> pathComponents = splitPath(key, '.');

        if (pathComponents.empty()) {
            LOG_ERROR("setValue called with empty key.", "Config");
            return;
        }

        toml::table* currentTable = &runtimeOverrides;

        for (size_t i = 0; i < pathComponents.size() - 1; ++i) {
            const std::string& segment = pathComponents[i];

            toml::node* node = currentTable->get(segment);

            if (node && node->is_table()) {
                currentTable = node->as_table();
            } else if (!node) {
                auto result_pair = currentTable->emplace<toml::table>(segment);
                toml::node& newNode = result_pair.first->second;

                if (newNode.is_table()) {
                    currentTable = newNode.as_table();
                } else {
                    LOG_ERROR(("Failed to emplace segment '" + segment + "' as a table.").c_str(), "Config");
                    return;
                }
            } else {
                LOG_ERROR(("Cannot create table path segment '" + segment + "' in key '" + key + "' because a non-table value already exists there.").c_str(), "Config");
                return;
            }
        }

        const std::string& lastSegment = pathComponents.back();
        currentTable->insert_or_assign(lastSegment, value);
        LOG_INFO(("Runtime value set for key: " + key).c_str(), "Config");
    }

    void shutdown();
private:
    toml::table runtimeOverrides;
    toml::table defaultConfigValues; // For hardcoded defaults
    std::optional<toml::table> primaryConfigTable; // Parsed primary file
    std::optional<toml::table> userConfigTable;    // Parsed user file
    // Maybe store file paths if needed for saving later
    std::string storedPrimaryPath;
    std::string storedUserPath; 

    mutable std::mutex configMutex_;

    template <typename T>
    std::optional<T> tryGetValueFromSource(const toml::table& source, const std::string& key) const {
        if (auto node = source.at_path(key)) {
            // node.value<T>() already returns std::optional<T>
            return node.value<T>();
        }
        return std::nullopt; // Key not found in this source
    }

    template <typename T>
    std::optional<T> tryGetValueFromSource(const std::optional<toml::table>& sourceOpt, const std::string& key) const {
        if (sourceOpt) {
            // Delegate to the non-optional version
            return tryGetValueFromSource<T>(*sourceOpt, key);
        }
        return std::nullopt; // Source itself doesn't exist
    }
};

}