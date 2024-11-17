#include "GameSettings.h"
#include "../Debug.h"
#include <filesystem>
#include <fstream>

bool GameSettings::LoadSettings(const std::string& filepath) {
    try {
        std::filesystem::path configPath = std::filesystem::path(filepath).parent_path();
        std::filesystem::create_directories(configPath);

        std::ifstream file(filepath);
        if (!file.is_open()) {
            DEBUG_ERROR("Failed to open settings file: ", filepath);
            return false;
        }

        nlohmann::json j;
        file >> j;
        return SettingsRegistry::Instance().LoadFromJson(j);
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Error loading settings: ", e.what());
        return false;
    }
}

bool GameSettings::SaveSettings(const std::string& filepath) const {
    try {
        std::filesystem::path configPath = std::filesystem::path(filepath).parent_path();
        std::filesystem::create_directories(configPath);

        std::ofstream file(filepath);
        file << SettingsRegistry::Instance().SaveToJson().dump(4);
        return true;
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Error saving settings: ", e.what());
        return false;
    }
} 