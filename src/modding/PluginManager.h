#pragma once

#include "Plugin.h"
#include "PluginLoader.h"
#include "Sandbox.h"
#include <unordered_map>
#include <filesystem>
#include <memory>
#include <string>

class PluginManager {
public:
    PluginManager();
    ~PluginManager();

    /* Plugin Management */
    bool LoadPlugin(const std::filesystem::path& pluginPath);
    void UnloadPlugin(const std::string& pluginName);
    void UnloadAllPlugins();
    void UpdatePlugins(float dt);

    /* Plugin Validation */
    bool ValidatePlugin(const std::filesystem::path& pluginPath) const;
    
    /* Plugin Access */
    std::shared_ptr<Plugin> GetPlugin(const std::string& name) const;
    std::vector<std::shared_ptr<Plugin>> GetPluginsByType(PluginType type) const;

private:
    std::shared_ptr<Plugin> LoadPluginFromPath(const std::filesystem::path& path);
    bool IsAPIVersionCompatible(const std::string& version) const;
    bool ValidateAssetStructure(const std::filesystem::path& assetsPath) const;
    bool IsAllowedAssetExtension(const std::string& ext) const;
    
    std::unordered_map<std::string, std::shared_ptr<Plugin>> m_plugins;
    std::unique_ptr<Sandbox> m_sandbox;
}; 