#include "PluginManager.h"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <nlohmann/json.hpp>

PluginManager::PluginManager()
    : m_sandbox(std::make_unique<Sandbox>())
{}

PluginManager::~PluginManager() {
    UnloadAllPlugins();
}

bool PluginManager::LoadPlugin(const std::filesystem::path& pluginPath) {
    try {
        // Validate plugin before loading
        if (!ValidatePlugin(pluginPath)) {
            std::cerr << "Plugin validation failed: " << pluginPath << std::endl;
            return false;
        }

        // Load plugin in sandbox environment
        if (!m_sandbox->IsPathSafe(pluginPath)) {
            std::cerr << "Plugin path security check failed: " << pluginPath << std::endl;
            return false;
        }

        // Create plugin instance (implementation depends on your plugin format)
        // This is where you'd load the DLL/SO file or script
        auto plugin = LoadPluginFromPath(pluginPath);
        
        if (!plugin) {
            std::cerr << "Failed to create plugin instance: " << pluginPath << std::endl;
            return false;
        }

        // Store plugin
        m_plugins[plugin->GetName()] = plugin;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading plugin: " << e.what() << std::endl;
        return false;
    }
}

void PluginManager::UpdatePlugins(float dt) {
    for (auto& [name, plugin] : m_plugins) {
        if (plugin && plugin->IsEnabled()) {
            try {
                plugin->Update(dt);
            }
            catch (const std::exception& e) {
                std::cerr << "Error updating plugin " << name << ": " << e.what() << std::endl;
                plugin->Unload();
            }
        }
    }
}

std::shared_ptr<Plugin> PluginManager::LoadPluginFromPath(const std::filesystem::path& path) {
    try {
        return PluginLoader::LoadPlugin(path);
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load plugin from " << path << ": " << e.what() << std::endl;
        return nullptr;
    }
}

void PluginManager::UnloadPlugin(const std::string& pluginName) {
    auto it = m_plugins.find(pluginName);
    if (it != m_plugins.end()) {
        it->second->Unload();
        m_plugins.erase(it);
    }
}

void PluginManager::UnloadAllPlugins() {
    for (auto& [name, plugin] : m_plugins) {
        if (plugin) {
            plugin->Unload();
        }
    }
    m_plugins.clear();
}

std::shared_ptr<Plugin> PluginManager::GetPlugin(const std::string& name) const {
    auto it = m_plugins.find(name);
    return (it != m_plugins.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<Plugin>> PluginManager::GetPluginsByType(PluginType type) const {
    std::vector<std::shared_ptr<Plugin>> result;
    for (const auto& [name, plugin] : m_plugins) {
        if (plugin && plugin->GetType() == type) {
            result.push_back(plugin);
        }
    }
    return result;
}

bool PluginManager::ValidatePlugin(const std::filesystem::path& pluginPath) const {
    try {
        // Check if file exists and has correct extension
        if (!std::filesystem::exists(pluginPath)) {
            std::cerr << "Plugin file does not exist: " << pluginPath << std::endl;
            return false;
        }

        if (pluginPath.extension() != PLUGIN_EXTENSION) {
            std::cerr << "Invalid plugin extension: " << pluginPath << std::endl;
            return false;
        }

        // Check manifest file
        auto manifestPath = pluginPath.parent_path() / "manifest.json";
        if (!std::filesystem::exists(manifestPath)) {
            std::cerr << "Missing manifest file for plugin: " << pluginPath << std::endl;
            return false;
        }

        // Parse and validate manifest
        std::ifstream manifestFile(manifestPath);
        nlohmann::json manifest;
        manifestFile >> manifest;

        // Check required manifest fields
        const std::vector<std::string> requiredFields = {
            "name", "version", "author", "description", "type", "api_version"
        };

        for (const auto& field : requiredFields) {
            if (!manifest.contains(field)) {
                std::cerr << "Missing required field in manifest: " << field << std::endl;
                return false;
            }
        }

        // Validate plugin type
        std::string pluginType = manifest["type"];
        if (pluginType != "asset" && pluginType != "gameplay" && pluginType != "interface") {
            std::cerr << "Invalid plugin type: " << pluginType << std::endl;
            return false;
        }

        // Check API version compatibility
        std::string apiVersion = manifest["api_version"];
        if (!IsAPIVersionCompatible(apiVersion)) {
            std::cerr << "Incompatible API version: " << apiVersion << std::endl;
            return false;
        }

        // Read plugin binary for security validation
        std::ifstream pluginFile(pluginPath, std::ios::binary);
        std::string pluginContent((std::istreambuf_iterator<char>(pluginFile)),
                                 std::istreambuf_iterator<char>());

        // Validate plugin code safety using sandbox
        if (!m_sandbox->ValidateCode(pluginContent)) {
            std::cerr << "Plugin code validation failed: " << pluginPath << std::endl;
            return false;
        }

        // Check plugin file size
        auto fileSize = std::filesystem::file_size(pluginPath);
        constexpr size_t MAX_PLUGIN_SIZE = 50 * 1024 * 1024; // 50MB
        if (fileSize > MAX_PLUGIN_SIZE) {
            std::cerr << "Plugin file size exceeds maximum allowed: " << pluginPath << std::endl;
            return false;
        }

        // Validate plugin assets directory structure if it exists
        auto assetsPath = pluginPath.parent_path() / "assets";
        if (std::filesystem::exists(assetsPath)) {
            if (!ValidateAssetStructure(assetsPath)) {
                std::cerr << "Invalid plugin assets structure: " << pluginPath << std::endl;
                return false;
            }
        }

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error validating plugin: " << e.what() << std::endl;
        return false;
    }
}

bool PluginManager::IsAPIVersionCompatible(const std::string& version) const {
    // Simple version comparison for now
    // Format: "major.minor"
    const std::string CURRENT_API_VERSION = "1.0";
    return version == CURRENT_API_VERSION;
}

bool PluginManager::ValidateAssetStructure(const std::filesystem::path& assetsPath) const {
    // Check for required asset directories
    const std::vector<std::string> requiredDirs = {
        "textures",
        "sounds",
        "models"
    };

    for (const auto& dir : requiredDirs) {
        auto dirPath = assetsPath / dir;
        if (std::filesystem::exists(dirPath) && !std::filesystem::is_directory(dirPath)) {
            return false;
        }
    }

    // Recursively check all files in assets directory
    for (const auto& entry : std::filesystem::recursive_directory_iterator(assetsPath)) {
        if (entry.is_regular_file()) {
            // Check if file extension is allowed
            std::string ext = entry.path().extension().string();
            if (!IsAllowedAssetExtension(ext)) {
                return false;
            }

            // Check if file size is within limits
            constexpr size_t MAX_ASSET_SIZE = 100 * 1024 * 1024; // 100MB
            if (entry.file_size() > MAX_ASSET_SIZE) {
                return false;
            }
        }
    }

    return true;
}

bool PluginManager::IsAllowedAssetExtension(const std::string& ext) const {
    const std::vector<std::string> allowedExtensions = {
        ".png", ".jpg", ".jpeg", ".wav", ".ogg", ".obj", ".fbx"
    };

    return std::find(allowedExtensions.begin(), allowedExtensions.end(), ext) 
           != allowedExtensions.end();
} 