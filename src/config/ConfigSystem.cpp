#include "config/ConfigSystem.h"

#include <filesystem>
#include <string>

namespace {
std::optional<toml::table> loadConfigFile(const std::string& filepath, const std::string& fileType) {
    LOG_INFO(("Attempting to load " + fileType + " config from: " + filepath).c_str(), "Config");

    if (!std::filesystem::exists(filepath)) {
        LOG_WARN((fileType + " config not found at: " + filepath + ". Ignoring.").c_str(), "Config");
        return std::nullopt;
    }

    try {
        toml::table parsedTable = toml::parse_file(filepath);
        LOG_INFO((fileType + " config loaded successfully from: " + filepath).c_str(), "Config");
        return parsedTable;
    } catch (const toml::parse_error& err) {
        // Log the specific TOML error details if possible/desired
        LOG_ERROR(("Failed to parse " + fileType + " config from: " + filepath + ". Error: " + std::string(err.what())).c_str(), "Config");
        return std::nullopt;
    } catch (const std::exception& e) {
        LOG_ERROR(("Unexpected error while parsing " + fileType + " config from: " + filepath + ". Error: " + std::string(e.what())).c_str(), "Config");
        return std::nullopt;
    }
}

void mergeTomlTables(toml::table& dest, const toml::table& src) {
    for (auto&& [key, src_node_ref] : src) {
        toml::node* dest_node = dest.get(key); 

        if (src_node_ref.is_table() && dest_node && dest_node->is_table()) { 
            mergeTomlTables(*dest_node->as_table(), *src_node_ref.as_table());
        } else {
            dest.insert_or_assign(key, src_node_ref);
        }
    }
}
}

namespace transity::config {

ConfigSystem::ConfigSystem() {
    initialize();
    LOG_INFO("Config system initialized", "Config");
}

ConfigSystem::~ConfigSystem() {
    LOG_INFO("Config system shutting down", "Config");
    defaultConfigValues.clear();
}

void ConfigSystem::initialize(const std::string& primaryConfigFilepath, const std::string& userConfigFilepath) {
    // 1. Load Defaults
    defaultConfigValues.clear();
    defaultConfigValues.insert_or_assign("windowWidth", 800);
    defaultConfigValues.insert_or_assign("windowHeight", 600);
    defaultConfigValues.insert_or_assign("windowTitle", "Transity");
    LOG_INFO("Default configuration loaded", "Config");

    // 2. Load Primary Config File
    primaryConfigTable.reset(); // Ensure it's clear before attempting load
    storedPrimaryPath = "";     // Clear stored path
    auto loadedPrimary = loadConfigFile(primaryConfigFilepath, "Primary");
    if (loadedPrimary) {
        primaryConfigTable = std::move(*loadedPrimary); // Move the table
        storedPrimaryPath = primaryConfigFilepath;      // Store path on success
    } else {
        // Log message already handled in loadConfigFile if file not found or parse error
        LOG_WARN("Using default configuration values as primary source.", "Config");
    }


    // 3. Load User Config File (Optional)
    userConfigTable.reset(); // Ensure it's clear
    storedUserPath = "";     // Clear stored path
    if (!userConfigFilepath.empty()) {
        auto loadedUser = loadConfigFile(userConfigFilepath, "User");
        if (loadedUser) {
            userConfigTable = std::move(*loadedUser); // Move the table
            storedUserPath = userConfigFilepath;      // Store path on success
        }
        // Log messages handled in loadConfigFile
    } else {
        LOG_INFO("No user config file specified. Skipping.", "Config");
    }

    // 4. Log Initialization Summary (Optional)
    // Could add a log message summarizing which files were successfully loaded
    LOG_INFO("Config system initialization complete.", "Config");
}

void ConfigSystem::shutdown() {
    LOG_INFO("Config system shutting down...", "Config");
    if (storedUserPath.empty()) {
        LOG_WARN("No user config path stored, cannot save runtime changes.", "Config");
        return;
    }

    toml::table tableToSave;
    if (userConfigTable) {
        tableToSave = *userConfigTable;
    }

    mergeTomlTables(tableToSave, runtimeOverrides);

    if (tableToSave.empty()) {
        LOG_INFO("No user or runtime configuration settings to save.", "Config");
        return;
    }

    LOG_INFO(("Attempting to save configuration to: " + storedUserPath).c_str(), "Config");
    try {
        std::ofstream outFile(storedUserPath);
        if (!outFile.is_open()) {
            LOG_ERROR(("Failed to open user config file for writing: " + storedUserPath).c_str(), "Config");
            return;
        }

        toml::toml_formatter formatter{ tableToSave };
        outFile << formatter;
        outFile.close();

        if (outFile.fail()) {
            LOG_ERROR(("Failed to write to user config file: " + storedUserPath).c_str(), "Config");
        } else {
            LOG_INFO(("Configuration saved to: " + storedUserPath).c_str(), "Config");
        }
    } catch (const std::exception& e) {
        LOG_ERROR(("Unexpected error while saving configuration: " + std::string(e.what())).c_str(), "Config");
    }
}

}