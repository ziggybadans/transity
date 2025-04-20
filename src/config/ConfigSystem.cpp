#include "config/ConfigSystem.hpp"

#include <filesystem>
#include <string>

namespace {
std::optional<toml::table> loadConfigFile(const std::string& filepath, const std::string& fileType) {
    LOG_INFO("Config", ("Attempting to load " + fileType + " config from: " + filepath).c_str());

    if (!std::filesystem::exists(filepath)) {
        LOG_WARN("Config", (fileType + " config not found at: " + filepath + ". Ignoring.").c_str());
        return std::nullopt;
    }

    try {
        toml::table parsedTable = toml::parse_file(filepath);
        LOG_INFO("Config", (fileType + " config loaded successfully from: " + filepath).c_str());
        return parsedTable;
    } catch (const toml::parse_error& err) {
        // Log the specific TOML error details if possible/desired
        LOG_ERROR("Config", ("Failed to parse " + fileType + " config from: " + filepath + ". Error: " + std::string(err.what())).c_str());
        return std::nullopt;
    } catch (const std::exception& e) {
        LOG_ERROR("Config", ("Unexpected error while parsing " + fileType + " config from: " + filepath + ". Error: " + std::string(e.what())).c_str());
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
    LOG_INFO("Config", "Config system initialized");
}

ConfigSystem::~ConfigSystem() {
    std::lock_guard<std::mutex> lock(configMutex_);
    LOG_INFO("Config", "Config system shutting down");
    defaultConfigValues.clear();
}

void ConfigSystem::initialize(const std::string& primaryConfigFilepath, const std::string& userConfigFilepath) {
    std::lock_guard<std::mutex> lock(configMutex_);

    // 1. Load Defaults
    defaultConfigValues.clear();
    defaultConfigValues.insert_or_assign("windowWidth", 800);
    defaultConfigValues.insert_or_assign("windowHeight", 600);
    defaultConfigValues.insert_or_assign("windowTitle", "Transity");
    LOG_INFO("Config", "Default configuration loaded");

    // 2. Load Primary Config File
    primaryConfigTable.reset(); // Ensure it's clear before attempting load
    storedPrimaryPath = "";     // Clear stored path
    auto loadedPrimary = loadConfigFile(primaryConfigFilepath, "Primary");
    if (loadedPrimary) {
        primaryConfigTable = std::move(*loadedPrimary); // Move the table
        storedPrimaryPath = primaryConfigFilepath;      // Store path on success
    } else {
        // Log message already handled in loadConfigFile if file not found or parse error
        LOG_WARN("Config", "Using default configuration values as primary source.");
    }


    // 3. Load User Config File (Optional)
    userConfigTable.reset(); // Ensure it's clear
    storedUserPath = "";     // Clear stored path
    if (!userConfigFilepath.empty()) {
        storedUserPath = userConfigFilepath;
        auto loadedUser = loadConfigFile(userConfigFilepath, "User");
        if (loadedUser) {
            userConfigTable = std::move(*loadedUser); // Move the table
        }
        // Log messages handled in loadConfigFile
    } else {
        LOG_INFO("Config", "No user config file specified. Skipping.");
    }

    // 4. Log Initialization Summary (Optional)
    // Could add a log message summarizing which files were successfully loaded
    LOG_INFO("Config", "Config system initialization complete.");
}

void ConfigSystem::shutdown() {
    std::lock_guard<std::mutex> lock(configMutex_);
    LOG_INFO("Config", "Config system shutting down...");
    if (storedUserPath.empty()) {
        LOG_WARN("Config", "No user config path stored, cannot save runtime changes.");
        return;
    }

    toml::table tableToSave;
    if (userConfigTable) {
        tableToSave = *userConfigTable;
    }

    mergeTomlTables(tableToSave, runtimeOverrides);

    if (tableToSave.empty()) {
        LOG_INFO("Config", "No user or runtime configuration settings to save.");
        return;
    }

    LOG_INFO("Config", ("Attempting to save configuration to: " + storedUserPath).c_str());
    try {
        std::ofstream outFile(storedUserPath);
        if (!outFile.is_open()) {
            LOG_ERROR("Config", ("Failed to open user config file for writing: " + storedUserPath).c_str());
            return;
        }

        toml::toml_formatter formatter{ tableToSave };
        outFile << formatter;
        outFile.close();

        if (outFile.fail()) {
            LOG_ERROR("Config", ("Failed to write to user config file: " + storedUserPath).c_str());
        } else {
            LOG_INFO("Config", ("Configuration saved to: " + storedUserPath).c_str());
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Config", ("Unexpected error while saving configuration: " + std::string(e.what())).c_str());
    }
}

}